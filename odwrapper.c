/*
 *
 */

#include "php_odus.h"
#include "odwrapper.h"

#include "od_def.h"
#include "od_hash.h"
#include "od_igbinary.h"
#include "zend_llist.h"

ZEND_EXTERN_MODULE_GLOBALS(odus);

void debug_igsd(od_igbinary_unserialize_data* igsd) {
	debug_buffer(igsd->buffer, igsd->buffer_size, igsd->buffer_offset);
}

void debug_igsd_info(od_igbinary_unserialize_data* igsd) {
	debug("buffer: %p size: %d offset: %d",igsd->buffer, igsd->buffer_size, igsd->buffer_offset);
}

ODUS_ARGINFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(arginfo_ctor, 0, 0, 2)
	ZEND_ARG_INFO(0, classname)
	ZEND_ARG_INFO(1, data)
ZEND_END_ARG_INFO()

// Static Variables

static zend_class_entry* od_wrapper_ce = NULL;
zend_object_handlers od_wrapper_object_handlers;

static zend_llist * memory_collection_list = NULL;

// Static Core Functions Definition

static zend_object_value od_wrapper_new(zend_class_entry *ce TSRMLS_DC);
static od_wrapper_object* od_wrapper_object_new(zend_class_entry *ce TSRMLS_DC);
static zend_object_value od_wrapper_register_object(od_wrapper_object* intern TSRMLS_DC);

static inline void init_od_wrapper(zval *object, od_igbinary_unserialize_data* parent_igsd, uint8_t* data_str, uint32_t data_len,
									char* class_name, int class_name_len, uint32_t member_num, uint32_t value_len, uint32_t offset);
static inline zval* new_od_wrapper(zval *object, od_igbinary_unserialize_data* parent_igsd, uint8_t* data_str, uint32_t data_len,
									char* class_name, int class_name_len, uint32_t member_num, uint32_t value_len, uint32_t offset);

static int od_wrapper_migrate(od_igbinary_unserialize_data *igsd, uint32_t version TSRMLS_DC);

static inline zend_llist* get_memory_collection_list();

OD_WRAPPER_METHOD(__construct)
{
	zval* data;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",&data)) {
		return;
	}

	SET_OD_REFCOUNT(data);

	if(data==NULL || data->type!=IS_STRING || Z_STRVAL_P(data)==NULL || Z_STRLEN_P(data)<=0) {
		od_error(E_ERROR,"parameter for the constructor of ODWrapper must be valid string");
	}

	zend_llist *collection_list = get_memory_collection_list();
	if (collection_list != NULL)
	{
		zend_llist_add_element(collection_list, &data);
	}

	od_igbinary_unserialize_data igsd;

	od_igbinary_unserialize_data_init(&igsd);

	igsd.buffer = Z_STRVAL_P(data);
	igsd.buffer_size = Z_STRLEN_P(data);
	igsd.buffer_offset=0;

	igsd.root_id = Z_STRVAL_P(data);
	
	//version check
	uint32_t version = -1;
	od_igbinary_unserialize_header(&igsd, &version TSRMLS_CC);

	od_wrapper_migrate(&igsd, version TSRMLS_CC);

	if (igsd.compact_strings) {
		// Will alloc memory for string table, don't forget to free it in destructor!
		od_igbinary_unserialize_init_string_table(&igsd TSRMLS_CC);
	}

	char* class_name;
	uint32_t class_name_len;

	uint32_t class_info_offset = igsd.buffer_offset;

	od_igbinary_unserialize_class_name(&igsd, od_igbinary_get_type(&igsd), &class_name, &class_name_len);

	int member_num = od_igbinary_get_member_num(&igsd,od_igbinary_get_type(&igsd));

	int value_len = od_igbinary_get_value_len(&igsd);

	init_od_wrapper(getThis(), &igsd, igsd.buffer + class_info_offset, igsd.buffer_offset - class_info_offset + value_len, class_name, class_name_len,
				member_num, value_len, igsd.buffer_offset - class_info_offset);
}

static zend_function_entry od_wrapper_functions[] = {
	PHP_ME(ODWrapper, __construct, arginfo_ctor, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	{ NULL, NULL, NULL}
};

// Copied Utilities Definition

static inline int od_wrapper_call_setter(zval *object, zval *member, zval *value TSRMLS_DC);
static inline zval *od_wrapper_call_getter(zval *object, zval *member TSRMLS_DC);

static inline zval *od_wrapper_call_issetter(zval *object, zval *member TSRMLS_DC);
static inline void od_wrapper_call_unsetter(zval *object, zval *member TSRMLS_DC);

static int od_get_property_guard(zend_object *zobj, zend_property_info *property_info, zval *member, zend_guard **pguard);

// Hookers Definition

static void od_wrapper_object_clone(void *object, void **clone_ptr TSRMLS_DC);
static void od_wrapper_object_dtor(void *object, zend_object_handle handle TSRMLS_DC);
static void od_wrapper_object_free_storage(void *object TSRMLS_DC);

static HashTable* od_wrapper_get_properties(zval *object TSRMLS_DC);
static zval *od_wrapper_read_property(zval *object, zval *member, int type TSRMLS_DC);
static zval **od_wrapper_get_property_ptr_ptr(zval *object, zval *member TSRMLS_DC);
static void od_wrapper_write_property(zval *object, zval *member, zval *value TSRMLS_DC);

static void od_wrapper_unset_property(zval *object, zval *member TSRMLS_DC);
static int od_wrapper_has_property(zval *object, zval *member, int has_set_exists TSRMLS_DC);

//no implemented
static zend_object_value od_wrapper_objects_clone_obj(zval *zobject TSRMLS_DC);
static int od_wrapper_compare_objects(zval *o1, zval *o2 TSRMLS_DC);


static zend_function *od_wrapper_get_method(zval **object_ptr, char *method_name, int method_len TSRMLS_DC);
// Static Utilities Definition

static inline void od_wrapper_lazy_init(zval* obj, od_wrapper_object* od_obj);

static zval* od_wrapper_unserialize(od_igbinary_unserialize_data *igsd);
extern int od_wrapper_skip_value(od_igbinary_unserialize_data *igsd);

void search_member(od_wrapper_object* od_obj, const char* member_name, uint32_t member_len, uint32_t hash, ODBucket** ret_bkt, member_pos* ret_pos);
static int search_property(od_wrapper_object* od_obj,zend_property_info *property_info, ODBucket** ret_bkt);
static void get_all_members(od_wrapper_object* od_obj);

static int array_contains_object(zval* val);
// Could not be static
uint8_t is_od_wrapper_obj_modified(od_wrapper_object* od_obj, uint8_t has_sleep, int* member_num_diff, struct hash_si * visited_od_wrappers);

static void od_wrapper_modify_property(zval** variable_ptr, zval* value, zend_property_info *property_info, od_wrapper_object* od_obj);

// Function Implementation

// Exported Functions

static int od_wrapper_migrate(od_igbinary_unserialize_data *igsd, uint32_t version TSRMLS_DC)
{
	uint32_t current_version = OD_IGBINARY_FORMAT_VERSION;
	if (version < current_version) {
		zval *val = NULL;
		uint8_t * old_buf = igsd->buffer;

		// Expect od_igbinary_unserialize handles old format correctly.
		MAKE_STD_ZVAL(val);
		od_igbinary_unserialize(igsd->buffer, igsd->buffer_size, &val TSRMLS_CC);

		od_igbinary_serialize(&igsd->buffer, &igsd->buffer_size, val TSRMLS_CC);

		// Redo unserializing header.
		igsd->buffer_offset = 0;

		uint32_t version = -1;
		od_igbinary_unserialize_header(igsd, &version TSRMLS_CC);

		// TODO: release old_buf;
	}
}

void collect_memory(TSRMLS_D) {
	zend_llist *collection_list = get_memory_collection_list();
	if (collection_list != NULL)
	{
		debug("[ODWRAPPER before collect memory (%d,%s,%s)] list num=%d \n", OD_LINE, OD_FUNCTION, OD_FILE, zend_llist_count(collection_list));
		#ifdef OD_DEBUG_MEM
		uint32_t before_collect_memory = zend_memory_usage(0);
		uint32_t before_real_memory = zend_memory_usage(1);
		#endif

		zend_llist_clean(collection_list);

		debug("[ODWRAPPER after collect memory (%d,%s,%s)] list num=%d \n", OD_LINE, OD_FUNCTION, OD_FILE, zend_llist_count(collection_list));
		#ifdef OD_DEBUG_MEM
		uint32_t after_collect_memory = zend_memory_usage(0);
		uint32_t after_real_memory = zend_memory_usage(1);
		debug("[ODWRAPPER memory collected (%d, %d, %d))]\n", before_collect_memory, after_collect_memory, after_collect_memory-before_collect_memory);
		debug("[ODWRAPPER real memory collected (%d, %d, %d))]\n", before_real_memory, after_real_memory, after_real_memory-before_real_memory);
		#endif
	}
}

static void collect_root_string(zval **root_string) {
	ZVAL_REFCOUNT(*root_string) = 1;
	zval_ptr_dtor(root_string);

	//zval_dtor(root_string);

}

zend_llist* get_memory_collection_list() {
	if (memory_collection_list == NULL) {
		if (ODUS_G(auto_collect_memory)) {
			memory_collection_list = emalloc(sizeof(zend_llist));
			zend_llist_init(memory_collection_list, sizeof(zval*),  (llist_dtor_func_t) collect_root_string, 0);
		}
	}
	return memory_collection_list;
}

void od_wrapper_init(TSRMLS_D)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, PHP_ODUS_OBJECT_NAME, od_wrapper_functions);
	ce.create_object = od_wrapper_new;

	od_wrapper_ce = zend_register_internal_class(&ce TSRMLS_CC);

	memcpy(&od_wrapper_object_handlers,
			zend_get_std_object_handlers(),
			sizeof(zend_object_handlers));

	od_wrapper_object_handlers.get_properties = od_wrapper_get_properties;
	od_wrapper_object_handlers.read_property = od_wrapper_read_property;
	od_wrapper_object_handlers.write_property = od_wrapper_write_property;
	od_wrapper_object_handlers.get_property_ptr_ptr = od_wrapper_get_property_ptr_ptr;

	od_wrapper_object_handlers.has_property = od_wrapper_has_property;
	od_wrapper_object_handlers.unset_property = od_wrapper_unset_property;

	od_wrapper_object_handlers.get_method = od_wrapper_get_method;

	od_wrapper_object_handlers.clone_obj = od_wrapper_objects_clone_obj;
	od_wrapper_object_handlers.compare_objects = od_wrapper_compare_objects;
}

void od_wrapper_shutdown(TSRMLS_D)
{
	od_wrapper_ce = NULL;
}

// Static Core Functions Definition

zend_object_value od_wrapper_new(zend_class_entry *ce TSRMLS_DC)
{
	od_wrapper_object *intern = od_wrapper_object_new(ce TSRMLS_CC);
	return od_wrapper_register_object(intern TSRMLS_CC);
}

od_wrapper_object* od_wrapper_object_new(zend_class_entry *ce TSRMLS_DC)
{
	od_wrapper_object* intern = NULL;

	intern = ecalloc(1, sizeof(od_wrapper_object));

	//ALLOC_HASHTABLE(intern->zo.properties);
	//zend_hash_init(intern->zo.properties, 0, NULL, ZVAL_PTR_DTOR, 0);

	//XXX
	//intern->zo.properties will not be used unless get_properties is called

	intern->zo.properties = NULL;

	intern->zo.ce = ce;
	intern->zo.guards = NULL;

	return intern;
}

zend_object_value od_wrapper_register_object(od_wrapper_object* intern TSRMLS_DC)
{

	zend_object_value rv;

	rv.handle = zend_objects_store_put(intern,
					od_wrapper_object_dtor,
					(zend_objects_free_object_storage_t)od_wrapper_object_free_storage,
					od_wrapper_object_clone TSRMLS_CC);

	/* does not support (EG(ze1_compatibility_mode)) */

	rv.handlers = (zend_object_handlers *) &od_wrapper_object_handlers;

	return rv;
}

void init_od_wrapper(zval *object, od_igbinary_unserialize_data* parent_igsd, uint8_t* data_str, uint32_t data_len,
					char* class_name, int class_name_len, uint32_t member_num, uint32_t value_len, uint32_t offset)
{
	// data_str will points to the start of class
	// data_str + offset will points to the start of the first member
	//XXX
	//value_len is not used currently
	(void)value_len;

	//TODO
	//can be more lazy

	if(data_str==NULL || data_len<2 || offset<0 || class_name==NULL || class_name_len<0 || member_num<0) {
		od_error(E_ERROR,"The serialized data must be valid serialized data for object");
		return;
	}

	debug("initial odwrapper for class %s",class_name);

	od_wrapper_object* od_obj = (od_wrapper_object*)zend_object_store_get_object(object TSRMLS_CC);

	od_igbinary_unserialize_data* igsd = &(od_obj->igsd);

	od_igbinary_unserialize_data_clone(igsd, parent_igsd);

	igsd->buffer = data_str;
	igsd->buffer_size = data_len;
	igsd->buffer_offset = offset;

	zend_class_entry* real_ce = zend_fetch_class(class_name,class_name_len,ZEND_FETCH_CLASS_DEFAULT);

	if(real_ce == NULL) {
		od_error(E_ERROR,"class '%s' doesn't exist",class_name);
		return;
	}

	od_obj->zo.ce = real_ce;

	od_obj->od_properties = NULL;
}

zval* new_od_wrapper(zval *object, od_igbinary_unserialize_data* parent_igsd, uint8_t* data_str, uint32_t data_len,
					char* class_name, int class_name_len, uint32_t member_num, uint32_t value_len, uint32_t offset)
{
	if(data_str==NULL || data_len<=0) return NULL;

	if(object==NULL) {
		MAKE_STD_ZVAL(object);
	}

	object->type = IS_OBJECT;
	object->value.obj = od_wrapper_new(od_wrapper_ce);

	init_od_wrapper(object,parent_igsd,data_str,data_len,class_name,class_name_len,member_num,value_len,offset);

	return object;
}

// Copied Utilities Implementation

void od_zval_ptr_dtor(zval **zval_ptr)
{
	if(!zval_ptr || !(*zval_ptr)) {
		return;
	}

	// The code that was here was copied verbatim from _zval_ptr_dtor() in
	// Zend/zend_execute_API.c which didn't compile in debug mode, since
	// it referred to ZEND_FILE_LINE_RELAY_CC and ZEND_FILE_LINE_CC macros
	// that were only available to PHP itself.
	//
	// Call zval_ptr_dtor_wrapper instead. It will invoke _zval_ptr_dtor()
	// internally.
	zval_ptr_dtor_wrapper(zval_ptr);
}

int od_wrapper_call_setter(zval *object, zval *member, zval *value TSRMLS_DC)
{
	zval *retval = NULL;
	int result;
	zend_class_entry *ce = Z_OBJCE_P(object);

	SEPARATE_ARG_IF_REF(member);
	value->refcount++;

	/* __set handler is called with two arguments:
	     property name
	     value to be set

	   it should return whether the call was successfull or not
	*/
	zend_call_method_with_2_params(&object, ce, &ce->__set, ZEND_SET_FUNC_NAME, &retval, member, value);

	zval_ptr_dtor(&member);
	zval_ptr_dtor(&value);

	if (retval) {
		result = i_zend_is_true(retval) ? SUCCESS : FAILURE;
		zval_ptr_dtor(&retval);
		return result;
	} else {
		return FAILURE;
	}
}

zval *od_wrapper_call_getter(zval *object, zval *member TSRMLS_DC)
{
	zval *retval = NULL;
	zend_class_entry *ce = Z_OBJCE_P(object);

	/* __get handler is called with one argument:
	      property name

	   it should return whether the call was successfull or not
	*/

	SEPARATE_ARG_IF_REF(member);

	zend_call_method_with_1_params(&object, ce, &ce->__get, ZEND_GET_FUNC_NAME, &retval, member);

	zval_ptr_dtor(&member);

	if (retval) {
		retval->refcount--;
	}

	return retval;
}

zval *od_wrapper_call_issetter(zval *object, zval *member TSRMLS_DC)
{
	zval *retval = NULL;
	zend_class_entry *ce = Z_OBJCE_P(object);

	/* __isset handler is called with one argument:
	      property name

	   it should return whether the property is set or not
	*/

	SEPARATE_ARG_IF_REF(member);

	zend_call_method_with_1_params(&object, ce, &ce->__isset, ZEND_ISSET_FUNC_NAME, &retval, member);

	zval_ptr_dtor(&member);

	return retval;
}

void od_wrapper_call_unsetter(zval *object, zval *member TSRMLS_DC)
{
	zend_class_entry *ce = Z_OBJCE_P(object);

	/* __unset handler is called with one argument:
	      property name
	*/

	SEPARATE_ARG_IF_REF(member);

	zend_call_method_with_1_params(&object, ce, &ce->__unset, ZEND_UNSET_FUNC_NAME, NULL, member);

	zval_ptr_dtor(&member);
}

int od_get_property_guard(zend_object *zobj, zend_property_info *property_info, zval *member, zend_guard **pguard)
{
	zend_property_info info;
	zend_guard stub;

	if (!property_info) {
		property_info = &info;
		info.name = Z_STRVAL_P(member);
		info.name_length = Z_STRLEN_P(member);
		info.h = zend_get_hash_value(Z_STRVAL_P(member), Z_STRLEN_P(member) + 1);
	}
	if (!zobj->guards) {
		ALLOC_HASHTABLE(zobj->guards);
		zend_hash_init(zobj->guards, 0, NULL, NULL, 0);
	} else if (zend_hash_quick_find(zobj->guards, property_info->name, property_info->name_length+1, property_info->h, (void **) pguard) == SUCCESS) {
		return SUCCESS;
	}
	stub.in_get = 0;
	stub.in_set = 0;
	stub.in_unset = 0;
	stub.in_isset = 0;
	return zend_hash_quick_add(zobj->guards, property_info->name, property_info->name_length+1, property_info->h, (void**)&stub, sizeof(stub), (void**) pguard);
}

// Hookers Implementation

void od_wrapper_object_clone(void *object, void **clone_ptr TSRMLS_DC)
{
	od_error(E_ERROR,"you should not call function %s",__FUNCTION__);

	od_wrapper_object *intern = (od_wrapper_object*) object;
	od_wrapper_object *clone = NULL;

	clone = od_wrapper_object_new(intern->zo.ce TSRMLS_CC);

	*clone_ptr = clone;
}

void od_wrapper_object_dtor(void *object, zend_object_handle handle TSRMLS_DC)
{
	(void)object;
}

void od_wrapper_object_free_storage(void *object TSRMLS_DC)
{
	od_wrapper_object *intern = (od_wrapper_object*) object;

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION == 1 && PHP_RELEASE_VERSION > 2) || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 1) || (PHP_MAJOR_VERSION > 5)
	zend_object_std_dtor(&intern->zo TSRMLS_CC);
#else
	if (intern->zo.guards)
	{
		zend_hash_destroy(intern->zo.guards);
		FREE_HASHTABLE(intern->zo.guards);
	}

	if (intern->zo.properties)
	{
		zend_hash_destroy(intern->zo.properties);
		FREE_HASHTABLE(intern->zo.properties);
	}
#endif

	if(intern->od_properties) {
		od_hash_deinit(&intern->od_properties);
		intern->od_properties = NULL;
	}

	efree(intern);
}

HashTable* od_wrapper_get_properties(zval *object TSRMLS_DC)
{
	od_wrapper_object* od_obj = (od_wrapper_object*)zend_object_store_get_object(object TSRMLS_CC);

	if(od_obj->zo.properties) return od_obj->zo.properties;

	OD_HASH_LAZY_INIT(od_obj->zo.properties);

	debug("@@@ DANGOURS!!!!!!!! in od_wrapper_get_properties for class '%s'",OD_CLASS_NAME(od_obj));

	if(!od_obj->od_properties) od_wrapper_lazy_init(object,od_obj);

	get_all_members(od_obj);

	ODBucket* bkt = NULL;

	uint32_t i;

	if(od_obj->od_properties && od_obj->od_properties->buckets) {
		//add new properties here
		for(i=0;i<od_obj->od_properties->size;i++) {
			bkt = od_obj->od_properties->buckets + i;

			if(OD_IS_NEW(*bkt) && bkt->data!=NULL) {

				((zval*)(bkt->data))->refcount ++;
				zend_hash_update(od_obj->zo.properties,(char*)bkt->key,bkt->key_len+1,(zval**)(&bkt->data),sizeof(zval*),NULL);
			}
		}
	}

	Bucket* p = od_obj->zo.ce->default_properties.pListHead;

	while(p!=NULL) {

		if(p->pData && !zend_hash_quick_exists(od_obj->zo.properties,p->arKey,p->nKeyLength,p->h)) {

			if(od_hash_find(od_obj->od_properties,p->arKey,p->nKeyLength-1,OD_HASH_VALUE(p->h),&bkt) == FAILURE && (!bkt || !OD_IS_OCCUPIED(*bkt))) {

				zend_hash_quick_add(od_obj->zo.properties,p->arKey,p->nKeyLength,p->h,p->pData,sizeof(zval*),NULL);

				(*((zval**)p->pData))->refcount ++;

			}
		}

		p = p->pListNext;
	}

	return od_obj->zo.properties;
}

zval *od_wrapper_read_property(zval *object, zval *member, int type TSRMLS_DC)
{
	zend_object *zobj;
	zval *tmp_member = NULL;
	zval **retval = NULL;
	zval *rv = NULL;
	zend_property_info *property_info;
	int silent;

	ODBucket* bkt = NULL;

	silent = (type == BP_VAR_IS);
	zobj = (zend_object*)zend_object_store_get_object(object);

	od_wrapper_object* od_obj = (od_wrapper_object*)zobj;

	if(!od_obj->od_properties) od_wrapper_lazy_init(object,od_obj);

 	if (member->type != IS_STRING) {
 		ALLOC_ZVAL(tmp_member);
		*tmp_member = *member;
		INIT_PZVAL(tmp_member);
		zval_copy_ctor(tmp_member);
		convert_to_string(tmp_member);
		member = tmp_member;
	}

 	OD_CALL_INFO;

	/* make zend_get_property_info silent if we have getter - we may want to use it */
	property_info = zend_get_property_info(zobj->ce, member, (zobj->ce->__get != NULL) TSRMLS_CC);

	if (!property_info || OD_FIND_PROPERTY == FAILURE) {

		zend_guard *guard;

		if (zobj->ce->__get &&
		    od_get_property_guard(zobj, property_info, member, &guard) == SUCCESS &&
		    !guard->in_get) {

			OD_SEARCH_PROPERTY(retval);

			if(!retval) {
				/* have getter - try with it! */
				ZVAL_ADDREF(object);

				guard->in_get = 1; /* prevent circular getting */
				rv = od_wrapper_call_getter(object, member TSRMLS_CC);
				guard->in_get = 0;

				if (rv) {
					retval = &rv;
					if (!rv->is_ref &&
					    (type == BP_VAR_W || type == BP_VAR_RW  || type == BP_VAR_UNSET)) {
						if (rv->refcount > 0) {
							zval *tmp = rv;

							ALLOC_ZVAL(rv);
							*rv = *tmp;
							zval_copy_ctor(rv);
							rv->is_ref = 0;
							rv->refcount = 0;
						}
						if (Z_TYPE_P(rv) != IS_OBJECT) {
							od_error(E_NOTICE, "Indirect modification of overloaded property %s::$%s has no effect", zobj->ce->name, Z_STRVAL_P(member));
						}
					}
				} else {
					retval = &EG(uninitialized_zval_ptr);
				}
				zval_ptr_dtor(&object);
			} else {
				if((*retval)->type == IS_ARRAY && !OD_IS_MODIFIED(*bkt) && array_contains_object(*retval)) {
					OD_SET_MODIFIED(*bkt);
				}
			}
		} else {

			if(!zobj->ce->__get) {

				OD_SEARCH_PROPERTY(retval);

				if(!retval)
				{
					retval = &EG(uninitialized_zval_ptr);
				} else {
					if((*retval)->type == IS_ARRAY && !OD_IS_MODIFIED(*bkt) && array_contains_object(*retval)) {
						OD_SET_MODIFIED(*bkt);
					}
				}
			}

			if (!silent && retval == &EG(uninitialized_zval_ptr)) {
					zend_error(E_NOTICE,"Undefined property: %s::$%s", zobj->ce->name, Z_STRVAL_P(member));
			}
		}
	} else {
		OD_SET_RETVAL(retval);
	}

	if(!retval) {
		retval = &EG(uninitialized_zval_ptr);
	}

	if (tmp_member) {
		(*retval)->refcount++;
		zval_ptr_dtor(&tmp_member);
		(*retval)->refcount--;
	}

	return *retval;
}


zval **od_wrapper_get_property_ptr_ptr(zval *object, zval *member TSRMLS_DC)
{
	zend_object *zobj;
	zval tmp_member;
	zval **retval;
	zend_property_info *property_info;

	ODBucket* bkt = NULL;

	zobj = (zend_object *)zend_object_store_get_object(object TSRMLS_CC);

	od_wrapper_object* od_obj = (od_wrapper_object*)zobj;

	if(!od_obj->od_properties) od_wrapper_lazy_init(object,od_obj);

 	if (member->type != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
	}

 	OD_CALL_INFO;

	property_info = zend_get_property_info(zobj->ce, member, (zobj->ce->__get != NULL) TSRMLS_CC);

	if (!property_info || OD_FIND_PROPERTY == FAILURE) {
		zval *new_zval;
		zend_guard *guard;

		bool is_unset = bkt && OD_IS_UNSET(*bkt);

		if (!zobj->ce->__get ||
			od_get_property_guard(zobj, property_info, member, &guard) != SUCCESS ||
			(property_info && guard->in_get)) {

			if(!zobj->ce->__get) {
				OD_SEARCH_PROPERTY(retval);

				if(retval == NULL) {
					new_zval = &EG(uninitialized_zval);

					/* od_error(E_NOTICE, "Undefined property: %s", Z_STRVAL_P(member)); */
					new_zval->refcount++;

					OD_UPDATE_PROPERTY(1,new_zval,&bkt);

					if(bkt && !is_unset) {
						OD_SET_NEW(*bkt);
					}
				} else {
					OD_SET_MODIFIED(*bkt);
				}
			} else {
				/* we don't have access controls - will just add it */
				new_zval = &EG(uninitialized_zval);

				/* od_error(E_NOTICE, "Undefined property: %s", Z_STRVAL_P(member)); */
				new_zval->refcount++;

				OD_UPDATE_PROPERTY(1,new_zval,&bkt);

				if(bkt && !is_unset) {
					OD_SET_NEW(*bkt);
				}
			}
		} else {
			OD_SEARCH_PROPERTY(retval);
			if(retval != NULL) {
				OD_SET_MODIFIED(*bkt);
			}
			else {
				/* we do have getter - fail and let it try again with usual get/set */
			}
		}

		//Will treat this
	} else {
		OD_SET_RETVAL(retval);
		// Will treat this as modification to existed key
		if(bkt) {
			OD_SET_MODIFIED(*bkt);
		}
	}

	if (member == &tmp_member) {
		zval_dtor(member);
	}
	return retval;
}

void od_wrapper_write_property(zval *object, zval *member, zval *value TSRMLS_DC)
{
	zend_object *zobj;
	zval *tmp_member = NULL;
	zval **variable_ptr;
	zend_property_info *property_info;

	ODBucket* bkt = NULL;

	zobj = (zend_object *)zend_object_store_get_object(object TSRMLS_CC);

	od_wrapper_object* od_obj = (od_wrapper_object*)zobj;

	if(!od_obj->od_properties) od_wrapper_lazy_init(object,od_obj);

 	if (member->type != IS_STRING) {
 		ALLOC_ZVAL(tmp_member);
		*tmp_member = *member;
		INIT_PZVAL(tmp_member);
		zval_copy_ctor(tmp_member);
		convert_to_string(tmp_member);
		member = tmp_member;
	}

 	OD_CALL_INFO;

	property_info = zend_get_property_info(zobj->ce, member, (zobj->ce->__set != NULL) TSRMLS_CC);

	if (property_info && OD_FIND_PROPERTY == SUCCESS) {
		OD_SET_RETVAL(variable_ptr);
		/* if we already have this value there, we don't actually need to do anything */
		if (*variable_ptr != value) {
			od_wrapper_modify_property(variable_ptr,value,property_info,od_obj);

			OD_SET_MODIFIED(*bkt);
		}
	} else {

		bool is_unset = bkt && OD_IS_UNSET(*bkt);

		OD_SEARCH_PROPERTY(variable_ptr);

		if(variable_ptr!=NULL && *variable_ptr!=NULL) {
			od_wrapper_modify_property(variable_ptr,value,property_info,od_obj);

			OD_SET_MODIFIED(*bkt);

		} else {
			int setter_done = 0;
			zend_guard *guard;

			if (zobj->ce->__set &&
			    od_get_property_guard(zobj, property_info, member, &guard) == SUCCESS &&
			    !guard->in_set) {
				ZVAL_ADDREF(object);
				guard->in_set = 1; /* prevent circular setting */
				if (od_wrapper_call_setter(object, member, value TSRMLS_CC) != SUCCESS) {
					/* for now, just ignore it - __set should take care of warnings, etc. */
				}
				setter_done = 1;
				guard->in_set = 0;
				zval_ptr_dtor(&object);
			}
			if (!setter_done && property_info) {
				zval **foo;

				/* if we assign referenced variable, we should separate it */
				value->refcount++;
				if (PZVAL_IS_REF(value)) {
					SEPARATE_ZVAL(&value);
				}

				OD_UPDATE_PROPERTY(1,value,&bkt);

				if(bkt && !is_unset) {
					OD_SET_NEW(*bkt);
				}
			}
		}
	}

	if (tmp_member) {
		zval_ptr_dtor(&tmp_member);
	}
}

void od_wrapper_unset_property(zval *object, zval *member TSRMLS_DC)
{
	zend_object *zobj;
	zval *tmp_member = NULL;
	zend_property_info *property_info;

	ODBucket* bkt = NULL;

	zobj = (zend_object*)zend_object_store_get_object(object);

	od_wrapper_object* od_obj = (od_wrapper_object*)zobj;

	if(!od_obj->od_properties) od_wrapper_lazy_init(object,od_obj);

 	if (member->type != IS_STRING) {
 		ALLOC_ZVAL(tmp_member);
		*tmp_member = *member;
		INIT_PZVAL(tmp_member);
		zval_copy_ctor(tmp_member);
		convert_to_string(tmp_member);
		member = tmp_member;
	}

 	OD_CALL_INFO;

	property_info = zend_get_property_info(zobj->ce, member, (zobj->ce->__unset != NULL) TSRMLS_CC);

	if (!property_info || OD_FIND_PROPERTY == FAILURE) {

		uint8_t success = 0;

		if(property_info) {
			zval** val;
			if(search_property(od_obj,property_info,&bkt) == SUCCESS) {

				if(bkt) {
					debug("unset key '%s'", property_info->name);
					if(bkt->data) {
						OD_HASH_FREE_VAL(*bkt);
					}
					OD_SET_MODIFIED(*bkt);
					success = 1;
				}
			}
		}

		if(!success) {
			zend_guard *guard;

			debug("user has unset: %s",zobj->ce->__unset? "yes":"no");

			if (zobj->ce->__unset &&
			    od_get_property_guard(zobj, property_info, member, &guard) == SUCCESS &&
			    !guard->in_unset) {
				debug("will call user defined __unset function for key '%s;",property_info->name);
				/* have unseter - try with it! */
				ZVAL_ADDREF(object);
				guard->in_unset = 1; /* prevent circular unsetting */
				od_wrapper_call_unsetter(object, member TSRMLS_CC);
				guard->in_unset = 0;
				zval_ptr_dtor(&object);
			}
		}
	} else {
		debug("unset key '%s'", property_info->name);
		if(bkt) {
			if(bkt->data) {
				OD_HASH_FREE_VAL(*bkt);
			}
			OD_SET_MODIFIED(*bkt);
		}
	}

	if (tmp_member) {
		zval_ptr_dtor(&tmp_member);
	}
}

int od_wrapper_has_property(zval *object, zval *member, int has_set_exists TSRMLS_DC)
{
	zend_object *zobj;
	int result;
	zval **value;
	zval *tmp_member = NULL;
	zend_property_info *property_info;

	ODBucket* bkt = NULL;

	zobj = (zend_object *)zend_object_store_get_object(object TSRMLS_CC);

	od_wrapper_object* od_obj = (od_wrapper_object*)zobj;
	if(!od_obj->od_properties) od_wrapper_lazy_init(object,od_obj);

	if (member->type != IS_STRING) {
		ALLOC_ZVAL(tmp_member);
		*tmp_member = *member;
		INIT_PZVAL(tmp_member);
		zval_copy_ctor(tmp_member);
		convert_to_string(tmp_member);
		member = tmp_member;
	}

 	OD_CALL_INFO;

	property_info = zend_get_property_info(zobj->ce, member, 1 TSRMLS_CC);

	int searched = 0;

	if (!property_info || ( OD_FIND_PROPERTY == FAILURE && (searched=1, search_property(od_obj,property_info,&bkt)) == FAILURE)) {
		zend_guard *guard;

		result = 0;
		if ((has_set_exists != 2) &&
		    zobj->ce->__isset &&
		    od_get_property_guard(zobj, property_info, member, &guard) == SUCCESS &&
		    !guard->in_isset) {
			zval *rv;

			/* have issetter - try with it! */
			ZVAL_ADDREF(object);
			guard->in_isset = 1; /* prevent circular getting */
			rv = od_wrapper_call_issetter(object, member TSRMLS_CC);
			if (rv) {
				result = zend_is_true(rv);
				zval_ptr_dtor(&rv);
				if (has_set_exists && result && !EG(exception) && zobj->ce->__get && !guard->in_get) {
					guard->in_get = 1;
					rv = od_wrapper_call_getter(object, member TSRMLS_CC);
					guard->in_get = 0;
					if (rv) {
						rv->refcount++;
						result = i_zend_is_true(rv);
						zval_ptr_dtor(&rv);
					}
				}
			}
			guard->in_isset = 0;
			zval_ptr_dtor(&object);
		}
	} else {

		OD_SET_RETVAL(value);

		if(searched && value && *value) {
			if((*value)->type == IS_ARRAY && !OD_IS_MODIFIED(*bkt) && array_contains_object(*value)) {
				OD_SET_MODIFIED(*bkt);
			}
		}

		switch (has_set_exists) {
		case 0:
			result = (Z_TYPE_PP(value) != IS_NULL);
			break;
		default:
			result = zend_is_true(*value);
			break;
		case 2:
			result = 1;
			break;
		}
	}

	if (tmp_member) {
		zval_ptr_dtor(&tmp_member);
	}
	return result;
}

zend_function *od_wrapper_get_method(zval **object_ptr, char *method_name, int method_len TSRMLS_DC)
{
	od_wrapper_object* od_obj = (od_wrapper_object*)zend_object_store_get_object(*object_ptr TSRMLS_CC);

	debug("$$$ in get_method, func name '%s' for class %s",method_name, OD_CLASS_NAME(od_obj));

	if(object_ptr && *object_ptr && strncmp("__wakeup",method_name,method_len)!=0) {
		od_wrapper_lazy_init(*object_ptr,NULL);
	}

	return std_object_handlers.get_method(object_ptr,method_name,method_len);
}

// Static Utilities Definition

uint8_t is_default(char* key, uint key_len, ulong hash, zval* val, HashTable* ht)
{
	//don't check inputs

	if(val == NULL) {
		return 0;
	}

	if(val->type == IS_OBJECT || val->type == IS_ARRAY ) {
		return 0;
	}

	zval** retval_p = NULL;

	if(zend_hash_quick_find(ht, key, key_len, hash, (void**)&retval_p) == FAILURE) {
		return 0;
	} else {
		if(retval_p && *retval_p) {
			zval* dval = *retval_p;

			if(val->type == dval->type) {
				switch(val->type) {
				case IS_NULL:
					return 1;
				case IS_BOOL:
				case IS_LONG:
					return Z_LVAL_P(val) == Z_LVAL_P(dval);
				case IS_DOUBLE:
					return Z_DVAL_P(val) == Z_DVAL_P(dval);
				case IS_STRING:
					return Z_STRLEN_P(val) == Z_STRLEN_P(dval) && (Z_STRLEN_P(val)==0 || (Z_STRVAL_P(val)[0]==Z_STRVAL_P(dval)[0] && strncmp(Z_STRVAL_P(val),Z_STRVAL_P(dval),Z_STRLEN_P(val))==0));
				default:
					return 0;
				}
			} else {
				return 0;
			}
		} else {
			return 0;
		}
	}

	return 0;
}

inline void od_wrapper_lazy_init(zval* obj, od_wrapper_object* od_obj)
{
	assert(obj);

	if(od_obj==NULL) {
		od_obj = (od_wrapper_object*)zend_object_store_get_object(obj TSRMLS_CC);
	}

	if(od_obj->od_properties) return;

	od_hash_init(&od_obj->od_properties,0);

	if(!od_obj->od_properties) {
		od_error(E_ERROR,"failed to initialize od_obj->od_properties");
		return;
	}

	//XXX
	//no igsd->buffer_offset points to the first member

	zend_update_class_constants(od_obj->zo.ce);

	if (od_obj->zo.ce != PHP_IC_ENTRY && zend_hash_exists(&od_obj->zo.ce->function_table, "__wakeup", sizeof("__wakeup"))) {

		zval f;
		zval* h = NULL;

		INIT_PZVAL(&f);
		ZVAL_STRINGL(&f, "__wakeup", sizeof("__wakeup") - 1, 0);
		call_user_function_ex(CG(function_table), &obj, &f, &h, 0, 0, 1, NULL TSRMLS_CC);

		if (h) {
			zval_ptr_dtor(&h);
		}
	}
}

zend_object_value od_wrapper_objects_clone_obj(zval *zobject TSRMLS_DC)
{
	od_error(E_ERROR,"you should not call function %s",__FUNCTION__);
	zend_object_value val = {0};
	return val;
}

int od_wrapper_compare_objects(zval *o1, zval *o2 TSRMLS_DC)
{
	if(o1 == NULL && o2 == NULL) {
		return 0;
	} else {
		if(o1 == NULL || o2 == NULL) {
			return 1;
		} else {
			if(IS_OD_WRAPPER(o1)) {
				(void)od_wrapper_get_properties(o1);
			}

			if(IS_OD_WRAPPER(o2)) {
				(void)od_wrapper_get_properties(o2);
			}

			zend_object *zobj1, *zobj2;

			zobj1 = (zend_object *)zend_object_store_get_object(o1);
			zobj2 = (zend_object *)zend_object_store_get_object(o2);

			if (zobj1->ce != zobj2->ce) {
				return 1; /* different classes */
			}
			return zend_compare_symbol_tables_i(zobj1->properties, zobj2->properties TSRMLS_CC);
		}
	}

}

int od_wrapper_skip_value(od_igbinary_unserialize_data *igsd)
{
	uint32_t len;

	od_igbinary_type t = (od_igbinary_type) od_igbinary_get_type(igsd TSRMLS_CC);

	switch (t) {
		case od_igbinary_type_object8:
		case od_igbinary_type_object16:
		case od_igbinary_type_object32:
		case od_igbinary_type_object_id8:
		case od_igbinary_type_object_id16:
		case od_igbinary_type_object_id32:
		{
			char* class_name;
			uint32_t class_name_len;

			od_igbinary_unserialize_class_name(igsd, t, &class_name, &class_name_len);

			od_igbinary_get_member_num(igsd,od_igbinary_get_type(igsd));

			len = od_igbinary_get_value_len(igsd);

			igsd->buffer_offset += len;
		}
			break;
		case od_igbinary_type_array8:
		case od_igbinary_type_array16:
		case od_igbinary_type_array32:

			if(od_igbinary_get_member_num(igsd,t)<0) {
				od_error(E_ERROR,"array size could not be less than zero");
				break;
			}

			len = od_igbinary_get_value_len(igsd);

			igsd->buffer_offset += len;

			break;
		case od_igbinary_type_string8:

			if (igsd->buffer_offset + 2 > igsd->buffer_size) {
				od_error(E_ERROR, "od_igbinary_unserialize_chararray: end-of-data");
				return 1;
			}

			len = od_igbinary_unserialize8(igsd);
			igsd->buffer_offset += len + 1;
			break;
		case od_igbinary_type_string16:

			if (igsd->buffer_offset + 3 > igsd->buffer_size) {
				od_error(E_ERROR, "od_igbinary_unserialize_chararray: end-of-data");
				return 1;
			}

			len = od_igbinary_unserialize16(igsd);
			igsd->buffer_offset += len + 1;

			break;
		case od_igbinary_type_string32:

			if (igsd->buffer_offset + 5 > igsd->buffer_size) {
				od_error(E_ERROR, "od_igbinary_unserialize_chararray: end-of-data");
				return 1;
			}

			len = od_igbinary_unserialize32(igsd);
			igsd->buffer_offset += len + 1;

			break;
		case od_igbinary_type_long8p:
		case od_igbinary_type_long8n:
		case od_igbinary_type_static_string_id8:
			igsd->buffer_offset +=1;
			break;
		case od_igbinary_type_long16p:
		case od_igbinary_type_long16n:
		case od_igbinary_type_static_string_id16:
			igsd->buffer_offset +=2;
			break;
		case od_igbinary_type_long32p:
		case od_igbinary_type_long32n:
		case od_igbinary_type_static_string_id32:
			igsd->buffer_offset +=4;
			break;
		case od_igbinary_type_long64p:
		case od_igbinary_type_long64n:
		case od_igbinary_type_double:
			igsd->buffer_offset +=8;
			break;

		case od_igbinary_type_null:
		case od_igbinary_type_bool_true:
		case od_igbinary_type_bool_false:
		case od_igbinary_type_string_empty:
			break;
		default:
			od_error(E_ERROR, "od_wrapper_skip_value: unknown type '%02x', position %zu", t, igsd->buffer_offset);
			break;
	}

	if (igsd->buffer_offset > igsd->buffer_size) {
		od_error(E_ERROR, "od_igbinary_unserialize_chararray: end-of-data");
		return 1;
	} else {
		return 0;
	}
}

zval* od_wrapper_unserialize(od_igbinary_unserialize_data *igsd)
{
	zval* val=NULL;

	od_igbinary_type t = (od_igbinary_type) od_igbinary_get_type(igsd TSRMLS_CC);

	switch (t) {
		case od_igbinary_type_object8:
		case od_igbinary_type_object16:
		case od_igbinary_type_object32:
		case od_igbinary_type_object_id8:
		case od_igbinary_type_object_id16:
		case od_igbinary_type_object_id32:
		{
			char* class_name;
			uint32_t class_name_len;

			uint32_t class_info_offset = igsd->buffer_offset-1;
			od_igbinary_unserialize_class_name(igsd, t, &class_name, &class_name_len);

			int member_num = od_igbinary_get_member_num(igsd,od_igbinary_get_type(igsd));

			int value_len = od_igbinary_get_value_len(igsd);
			MAKE_STD_ZVAL(val);
			val = new_od_wrapper(val, igsd, igsd->buffer + class_info_offset, igsd->buffer_offset - class_info_offset + value_len, class_name, class_name_len, member_num, value_len, igsd->buffer_offset - class_info_offset);

			igsd->buffer_offset += value_len;
		}
			break;
		case od_igbinary_type_array8:
		case od_igbinary_type_array16:
		case od_igbinary_type_array32:
		{
			uint32_t i;

			char *key;
			uint32_t key_len = 0;
			long key_index = 0;

			od_igbinary_type key_type;

			HashTable *h;

			uint32_t n = od_igbinary_get_member_num(igsd,t);

			if(n<0) {
				od_error(E_ERROR,"array size could not be less than zero");
				break;
			}

			int value_len = od_igbinary_get_value_len(igsd);

			MAKE_STD_ZVAL(val);
			Z_TYPE_P(val) = IS_ARRAY;
			ALLOC_HASHTABLE(Z_ARRVAL_P(val));
			zend_hash_init(Z_ARRVAL_P(val), n + 1, NULL, ZVAL_PTR_DTOR, 0);

			/* empty array */
			if (n == 0) {
				break;
			}

			h = HASH_OF(val);

			for (i = 0; i < n; i++) {

				if(od_igbinary_unserialize_get_key(igsd,&key,&key_len,&key_index)){
					break;
				}

				//debug("parse value for key %s",key);

				zval* v = od_wrapper_unserialize(igsd);

				if (key) {
					zend_symtable_update(h, key, key_len + 1, &v, sizeof(v), NULL);
				} else {
					zend_hash_index_update(h, key_index, &v, sizeof(v), NULL);
				}
			}
		}
			break;
		case od_igbinary_type_string8:
		case od_igbinary_type_string16:
		case od_igbinary_type_string32:
		{
			char *tmp_chararray;
			uint32_t tmp_uint32_t;

			if (od_igbinary_unserialize_chararray(igsd, t, &tmp_chararray, &tmp_uint32_t TSRMLS_CC)) {
				break;
			}

			MAKE_STD_ZVAL(val);
			Z_TYPE_P(val) = IS_STRING;
			Z_STRVAL_P(val) = tmp_chararray;
			Z_STRLEN_P(val) = tmp_uint32_t;
			SET_OD_REFCOUNT(val);
		}
			break;
		case od_igbinary_type_string_empty:
			MAKE_STD_ZVAL(val);
			ZVAL_EMPTY_STRING(val);
			break;
		case od_igbinary_type_static_string_id8:
		case od_igbinary_type_static_string_id16:
		case od_igbinary_type_static_string_id32:
		{
			char *tmp_chararray;
			uint32_t tmp_uint32_t;

			if (od_igbinary_unserialize_static_string(igsd, t, &tmp_chararray, &tmp_uint32_t TSRMLS_CC)) {
				break;
			}

			MAKE_STD_ZVAL(val);
			Z_TYPE_P(val) = IS_STRING;
			Z_STRVAL_P(val) = tmp_chararray;
			Z_STRLEN_P(val) = tmp_uint32_t;
			SET_OD_REFCOUNT(val);
		}
			break;
		case od_igbinary_type_long8p:
		case od_igbinary_type_long8n:
		case od_igbinary_type_long16p:
		case od_igbinary_type_long16n:
		case od_igbinary_type_long32p:
		case od_igbinary_type_long32n:
		case od_igbinary_type_long64p:
		case od_igbinary_type_long64n:
		{
			long tmp_long;
			if (od_igbinary_unserialize_long(igsd, t, &tmp_long TSRMLS_CC)) {
				break;
			}
			MAKE_STD_ZVAL(val);
			ZVAL_LONG(val, tmp_long);
		}
			break;
		case od_igbinary_type_null:
			MAKE_STD_ZVAL(val);
			ZVAL_NULL(val);
			break;
		case od_igbinary_type_bool_false:
			MAKE_STD_ZVAL(val);
			ZVAL_BOOL(val, 0);
			break;
		case od_igbinary_type_bool_true:
			MAKE_STD_ZVAL(val);
			ZVAL_BOOL(val, 1);
			break;
		case od_igbinary_type_double:
		{
			double tmp_double;
			if (od_igbinary_unserialize_double(igsd, t, &tmp_double TSRMLS_CC)) {
				break;
			}
			MAKE_STD_ZVAL(val);
			ZVAL_DOUBLE(val, tmp_double);
		}
			break;
		default:
			od_error(E_ERROR, "od_wrapper_unserialize: unknown type '%02x', position %zu", t, igsd->buffer_offset);
			break;
	}

	if(val!=NULL)
	{
		val->refcount ++;
	}

	return val;
}

void search_member(od_wrapper_object* od_obj, const char* member_name, uint32_t member_len, uint32_t hash, ODBucket** ret_bkt, member_pos* ret_pos)
{
	if(!ret_bkt && !ret_pos) {
		od_error(E_ERROR, "at least one of ret_bkt or ret_pos should not be null");
		return;
	}

	if(ret_bkt) *ret_bkt = NULL;

	if(ret_pos) {
		ret_pos->key_start_offset = 0;
		ret_pos->val_start_offset = 0;
		ret_pos->val_end_offset = 0;
	}

	char* name = NULL;
	uint32_t len = 0;
	long index;

	zval* member = NULL;

	uint32_t start_offset = 0;

	od_igbinary_unserialize_data* igsd = &od_obj->igsd;

	//assert
	//od_obj->igsd->offset will always points to the first member now

	uint32_t first_key_offset = igsd->buffer_offset;

	while(igsd->buffer_offset < igsd->buffer_size)
	{
		start_offset = igsd->buffer_offset;

		od_igbinary_unserialize_get_key(igsd, &name, &len, &index);

		if(name==NULL || len==0) {
			od_error(E_ERROR, "key for object could not be null");
		}

		if((member_len == len && member_name[0]==name[0] && memcmp(member_name,name,len)==0)) {

			if(ret_pos) ret_pos->key_start_offset = start_offset;
			if(ret_pos) ret_pos->val_start_offset = igsd->buffer_offset;

			if(ret_bkt) {
				member = od_wrapper_unserialize(igsd);

				od_hash_update(od_obj->od_properties,name,len,hash,0,member,ret_bkt);
			} else {
				od_wrapper_skip_value(igsd);
			}

			if(ret_pos) {
				ret_pos->val_end_offset = igsd->buffer_offset;
			}

			break;

		} else {
			od_wrapper_skip_value(igsd);
		}
	}

	igsd->buffer_offset = first_key_offset;
}

int search_property(od_wrapper_object* od_obj,zend_property_info *property_info, ODBucket** ret_bkt)
{
	if(ret_bkt) {

		if(*ret_bkt) {
			// the property is unset
			if(OD_IS_UNSET(**ret_bkt)) {
				*ret_bkt = NULL;
				return FAILURE;
			}
		}

		*ret_bkt = NULL;
	}

	if(od_obj && property_info && ret_bkt)
	{
		debug("search in blob data for member '%s' with len %d and hash %u od hash: %u",property_info->name,property_info->name_length,property_info->h,OD_HASH_VALUE(property_info->h));
		search_member(od_obj,property_info->name, property_info->name_length, OD_HASH_VALUE(property_info->h), ret_bkt, NULL);
	}

	if(ret_bkt && !(*ret_bkt)) {

		debug("will search in default properties for member '%s' with len '%d' hash: %u, od hash: %u",property_info->name,property_info->name_length,property_info->h,OD_HASH_VALUE(property_info->h));

		zval** retval_p = NULL;

		if(zend_hash_quick_find(&od_obj->zo.ce->default_properties,property_info->name,property_info->name_length + 1,property_info->h,(void**)(&retval_p))==SUCCESS) {
			debug("found in default property");
			if(retval_p && *retval_p) {

				OD_UPDATE_PROPERTY(1,*retval_p,ret_bkt);

				(*retval_p)->refcount ++;

				if(ret_bkt && *ret_bkt) {
					OD_SET_NEW(**ret_bkt);
					OD_SET_DEFAULT(**ret_bkt);
				}
			}
		}
	}

	return (ret_bkt && *ret_bkt)? SUCCESS:FAILURE;
}

void get_all_members(od_wrapper_object* od_obj)
{
	char* name = NULL;
	uint32_t len = 0;
	long index;

	zval* member = NULL;

	ODBucket* bkt;

	ulong p_hash;
	uint32_t o_hash;

	od_igbinary_unserialize_data* igsd = &od_obj->igsd;

	//assert
	//od_obj->igsd->offset will always points to the first member now
	uint32_t first_key_offset = igsd->buffer_offset;

	while(igsd->buffer_offset < igsd->buffer_size)
	{
		od_igbinary_unserialize_get_key(igsd, &name, &len, &index);

		if(name==NULL || len==0) {
			od_error(E_ERROR, "key for object could not be null");
		}
		p_hash = zend_get_hash_value(name,len+1);
		o_hash = OD_HASH_VALUE(p_hash);

		if(od_hash_find(od_obj->od_properties,name,len,o_hash,&bkt) == SUCCESS) {
			((zval*)(bkt->data))->refcount ++;
			zend_hash_quick_update(od_obj->zo.properties,name,len+1,p_hash,(zval**)(&bkt->data),sizeof(zval*),NULL);

			od_wrapper_skip_value(igsd);
		} else {

			if(bkt && OD_IS_UNSET(*bkt)) {

				zend_hash_del_key_or_index(od_obj->zo.properties,name,len+1,p_hash,HASH_DEL_INDEX);

				od_wrapper_skip_value(igsd);
			} else {
				member = od_wrapper_unserialize(igsd);

				if(member) {
					member->refcount ++;
					zend_hash_quick_update(od_obj->zo.properties,name,len+1,p_hash,&member,sizeof(zval*),NULL);
				}
			}
		}
	}
	igsd->buffer_offset = first_key_offset;
}

int array_contains_object(zval* val)
{
	if(!val || val->type != IS_ARRAY) {
		return 0;
	}

	HashTable* ht = val->value.ht;

	if(!ht) {
		return 0;
	} else {
		Bucket* p = ht->pListHead;

		while(p != NULL) {

			zval* data = (zval*)p->pDataPtr;

			if(data) {
				if(data->type == IS_OBJECT) {
					return 1;
				} else if(data->type == IS_ARRAY) {
					return array_contains_object(data);
				}
			}

			p = p->pListNext;
		}
	}

	return 0;
}

uint8_t is_od_wrapper_obj_modified(od_wrapper_object* od_obj,uint8_t has_sleep, int* member_num_diff, struct hash_si * visited_od_wrappers)
{
	if(member_num_diff) *member_num_diff=0;

	if(!od_obj) {
		debug("return because od_obj is null");
		return 0;
	}

	ODHashTable* ht = od_obj->od_properties;
	ODBucket* bkt = NULL;

	if(ht==NULL) {
		debug("return because ht is null");
		return 0;
	}

	int num_diff =0;

	zval* val = NULL;
	uint8_t modified = 0;
	uint32_t i;
	uint32_t found = 0;
	uint32_t *found_p = &found;

	// Track ODWrappers we have already visited so circular references
	// don't drag us into an infinite loop and cause segfault.
	found = hash_si_size(visited_od_wrappers); /* just some arbitrary value */
	hash_si_insert(visited_od_wrappers, (char *) od_obj, sizeof(od_wrapper_object *), found);

	if (ht->used > 0) {
		for (i = 0; i < ht->size; i++) {
			bkt = ht->buckets + i;
			if (OD_IS_OCCUPIED(*bkt)) {

				val = (zval*) (bkt->data);

				if (val == NULL) {
					//unset value
					if (!OD_IS_NEW(*bkt) && OD_IS_MODIFIED(*bkt)) {

						debug("check -> existed key %s is unset", bkt->key);

						modified = 1;

						num_diff--;
					}

				} else {

					if (has_sleep && !OD_IS_SLEEP(*bkt)) {
						if (!OD_IS_NEW(*bkt)) {

							debug("check -> existed key %s is sleep out", bkt->key);

							modified = 1;
							OD_SET_MODIFIED(*bkt);

							num_diff--;
						}

					} else {
						if (OD_IS_NEW(*bkt) && !(ODUS_G(remove_default) && !OD_IS_MODIFIED(*bkt) && OD_IS_DEFAULT(*bkt))) {

							debug("check -> key %s is new", bkt->key);

							modified = 1;

							num_diff++;
						}

						if (IS_OD_WRAPPER(val)) {

							od_wrapper_object* sub_od_obj = (od_wrapper_object*) zend_object_store_get_object(val);

							if (hash_si_find(visited_od_wrappers, (char *) sub_od_obj, sizeof(od_wrapper_object *), found_p) == 1 /* not found */) {
																
								if (!is_od_wrapper_obj_modified(sub_od_obj, 0, NULL, visited_od_wrappers)) {

									OD_RESET_MODIFIED(*bkt);

								} else {
									modified = 1;
									OD_SET_MODIFIED(*bkt);
								}
							}
							else {
								debug("is_od_wrapper_obj_modified() already visited %s", bkt->key);
							}
						} else {
							if (OD_IS_MODIFIED(*bkt) || val->type == IS_OBJECT) {

								modified = 1;
								OD_SET_MODIFIED(*bkt);
							}

						}

					}
				}
			}
		}
	}

	if(member_num_diff) {
		*member_num_diff = num_diff;
	}

	return modified;
}

void od_wrapper_modify_property(zval** variable_ptr, zval* value, zend_property_info *property_info, od_wrapper_object* od_obj)
{
	/* if we are assigning reference, we shouldn't move it, but instead assign variable
	   to the same pointer */

	if (PZVAL_IS_REF(*variable_ptr)) {
		zval garbage = **variable_ptr; /* old value should be destroyed */

		/* To check: can't *variable_ptr be some system variable like error_zval here? */
		(*variable_ptr)->type = value->type;
		(*variable_ptr)->value = value->value;
		if (value->refcount>0) {
			zval_copy_ctor(*variable_ptr);
		}

		zval_dtor(&garbage);
	} else {
		zval *garbage = *variable_ptr;

		/* if we assign referenced variable, we should separate it */
		value->refcount++;
		if (PZVAL_IS_REF(value)) {
			SEPARATE_ZVAL(&value);
		}
		*variable_ptr = value;
		zval_ptr_dtor(&garbage);
	}
}

