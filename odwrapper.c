/*
 *
 */

#include "php.h"
#include "zend.h"
#include "zend_globals.h"
#include "zend_interfaces.h"
#include "zend_exceptions.h"
#include "php_streams.h"
#include "zend_object_handlers.h"
#include "ext/standard/php_var.h"
#include "ext/standard/php_smart_str.h"
#include "ext/standard/php_incomplete_class.h"

#include "php_odus.h"
#include "odwrapper.h"

#include "od_igbinary.h"

//FIXME
//to be deleted start
void debug_igsd(od_igbinary_unserialize_data* igsd) {
	debug_buffer(igsd->buffer, igsd->buffer_size, igsd->buffer_offset);
}

void debug_igsd_info(od_igbinary_unserialize_data* igsd) {
	debug("buffer: %p size: %d offset: %d",igsd->buffer, igsd->buffer_size, igsd->buffer_offset);
}
//to be deleted end

ODUS_ARGINFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(arginfo_ctor, 0, 0, 2)
	ZEND_ARG_INFO(0, classname)
	ZEND_ARG_INFO(1, data)
ZEND_END_ARG_INFO()

// Static Variables

static zend_class_entry* od_wrapper_ce = NULL;
zend_object_handlers od_wrapper_object_handlers;

// Static Core Functions Definition

static zend_object_value od_wrapper_new(zend_class_entry *ce TSRMLS_DC);
static od_wrapper_object* od_wrapper_object_new(zend_class_entry *ce TSRMLS_DC);
static zend_object_value od_wrapper_register_object(od_wrapper_object* intern TSRMLS_DC);

static inline void init_od_wrapper(zval *object, uint8_t* data_str, size_t data_len, char* class_name, int class_name_len, size_t member_num, size_t value_len, size_t offset);
static inline zval* new_od_wrapper(zval *object, uint8_t* data_str, size_t data_len, char* class_name, int class_name_len, size_t member_num, size_t value_len, size_t offset);

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

	od_igbinary_unserialize_data igsd;

	igsd.buffer = Z_STRVAL_P(data);
	igsd.buffer_size = Z_STRLEN_P(data);
	igsd.buffer_offset=0;

	//version check
	od_igbinary_unserialize_header(&igsd);

	char* class_name;
	size_t class_name_len;

	size_t class_info_offset = igsd.buffer_offset;

	od_igbinary_unserialize_chararray(&igsd, od_igbinary_get_type(&igsd), &class_name, &class_name_len);

	int member_num = od_igbinary_get_member_num(&igsd,od_igbinary_get_type(&igsd));

	int value_len = od_igbinary_get_value_len(&igsd);

	init_od_wrapper(getThis(),igsd.buffer + class_info_offset, igsd.buffer_size - class_info_offset, class_name, class_name_len, member_num, value_len, igsd.buffer_offset - class_info_offset);
}

static zend_function_entry od_wrapper_functions[] = {
	PHP_ME(ODWrapper, __construct, arginfo_ctor, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	{ NULL, NULL, NULL}
};

// Copied Utilities Definition

static Bucket *od_wrapper_hash_apply_deleter(HashTable *ht, Bucket *p);
void od_wrapper_hash_apply_with_argument(HashTable *ht, apply_func_arg_t apply_func, void *argument TSRMLS_DC);

static inline int od_wrapper_call_setter(zval *object, zval *member, zval *value TSRMLS_DC);
static inline zval *od_wrapper_call_getter(zval *object, zval *member TSRMLS_DC);

static inline zval *od_wrapper_call_issetter(zval *object, zval *member TSRMLS_DC);
static inline void od_wrapper_call_unsetter(zval *object, zval *member TSRMLS_DC);

static int od_get_property_guard(zend_object *zobj, zend_property_info *property_info, zval *member, zend_guard **pguard);

static void od_zval_ptr_dtor(zval **zval_ptr);

// Hookers Definition

static void od_wrapper_object_clone(void *object, void **clone_ptr TSRMLS_DC);
static void od_wrapper_object_dtor(void *object, zend_object_handle handle TSRMLS_DC);
static void od_wrapper_object_free_storage(void *object TSRMLS_DC);

static int od_wrapper_count_elements_t(zval *object, long *count TSRMLS_DC);

static HashTable* od_wrapper_get_properties(zval *object TSRMLS_DC);
static zval *od_wrapper_read_property(zval *object, zval *member, int type TSRMLS_DC);
static zval **od_wrapper_get_property_ptr_ptr(zval *object, zval *member TSRMLS_DC);
static void od_wrapper_write_property(zval *object, zval *member, zval *value TSRMLS_DC);

static void od_wrapper_unset_property(zval *object, zval *member TSRMLS_DC);
static int od_wrapper_has_property(zval *object, zval *member, int has_set_exists TSRMLS_DC);

static zend_function *od_wrapper_get_method(zval **object_ptr, char *method_name, int method_len TSRMLS_DC);
// Static Utilities Definition

static inline void od_wrapper_lazy_init(zval* obj, od_wrapper_object* od_obj);

static zval* od_wrapper_unserialize(od_igbinary_unserialize_data *igsd, zend_uchar skip);

static void search_member(od_wrapper_object* od_obj, zend_property_info *property_info, zval*** retval_p);
static int search_property(od_wrapper_object* od_obj,zend_property_info *property_info, zval*** retval_p);
static void get_all_members(od_wrapper_object* od_obj);

static int od_wrapper_move_obj(void* bkt, void* arg);

static int array_contains_object(zval* val);
// Could not be static
int check_od_wrapper_ob_status(zval* obj);

static inline int od_wrapper_exists_property(HashTable* ht, zend_property_info *property_info);

static inline void od_wrapper_del_property(HashTable* ht, zend_property_info *property_info);

static inline void od_wrapper_add_property_ex(HashTable** p_ht, char* name, int len, ulong h, zval** value);

static inline void od_wrapper_add_property(HashTable** p_ht, zend_property_info *property_info, zval** value);
static inline void od_wrapper_add_modified_property(od_wrapper_object* od_obj, zend_property_info *property_info, zval** value);

static inline void od_wrapper_add_obj_property(od_wrapper_object* od_obj, zend_property_info *property_info, zval** value);

static void od_wrapper_modify_property(zval** variable_ptr, zval* value, zend_property_info *property_info, od_wrapper_object* od_obj);

// Function Implementation

// Exported Functions

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

	od_wrapper_object_handlers.count_elements = od_wrapper_count_elements_t;

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

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION == 1 && PHP_RELEASE_VERSION > 2) || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 1) || (PHP_MAJOR_VERSION > 5)
	zend_object_std_init(&intern->zo, ce TSRMLS_CC);
#else
	ALLOC_HASHTABLE(intern->zo.properties);
	zend_hash_init(intern->zo.properties, 0, NULL, ZVAL_PTR_DTOR, 0);

	intern->zo.ce = ce;
	intern->zo.guards = NULL;
#endif

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

void init_od_wrapper(zval *object, uint8_t* data_str, size_t data_len, char* class_name, int class_name_len, size_t member_num, size_t value_len, size_t offset)
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

	igsd->buffer = data_str;
	igsd->buffer_size = data_len;
	igsd->buffer_offset = offset;

	zend_class_entry* real_ce = zend_fetch_class(class_name,class_name_len,ZEND_FETCH_CLASS_DEFAULT);

	if(real_ce == NULL) {
		od_error(E_ERROR,"class '%s' doesn't exist",class_name);
		return;
	}

	od_obj->zo.ce = real_ce;

	od_obj->initialized = 0;
	od_obj->all_get = 0;
}

zval* new_od_wrapper(zval *object, uint8_t* data_str, size_t data_len, char* class_name, int class_name_len, size_t member_num, size_t value_len, size_t offset)
{
	if(data_str==NULL || data_len<=0) return NULL;

	if(object==NULL) {
		MAKE_STD_ZVAL(object);
	}

	object->type = IS_OBJECT;
	object->value.obj = od_wrapper_new(od_wrapper_ce);

	init_od_wrapper(object,data_str,data_len,class_name,class_name_len,member_num,value_len,offset);

	return object;
}

// Copied Utilities Implementation

void od_zval_ptr_dtor(zval **zval_ptr)
{
	if(!zval_ptr || !(*zval_ptr)) {
		return;
	}

#if DEBUG_ZEND>=2
	printf("Reducing refcount for %x (%x):  %d->%d\n", *zval_ptr, zval_ptr, (*zval_ptr)->refcount, (*zval_ptr)->refcount-1);
#endif
	(*zval_ptr)->refcount--;
	if ((*zval_ptr)->refcount==0) {
		zval_dtor(*zval_ptr);
		safe_free_zval_ptr_rel(*zval_ptr ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_CC);
	} else if ((*zval_ptr)->refcount == 1) {
		if ((*zval_ptr)->type == IS_OBJECT) {
			TSRMLS_FETCH();

			if (EG(ze1_compatibility_mode)) {
				return;
			}
		}
		(*zval_ptr)->is_ref = 0;
	}
}

Bucket *od_wrapper_hash_apply_deleter(HashTable *ht, Bucket *p)
{
	Bucket *retval;

	HANDLE_BLOCK_INTERRUPTIONS();
	if (p->pLast) {
		p->pLast->pNext = p->pNext;
	} else {
		uint nIndex;

		nIndex = p->h & ht->nTableMask;
		ht->arBuckets[nIndex] = p->pNext;
	}
	if (p->pNext) {
		p->pNext->pLast = p->pLast;
	} else {
		/* Nothing to do as this list doesn't have a tail */
	}

	if (p->pListLast != NULL) {
		p->pListLast->pListNext = p->pListNext;
	} else {
		/* Deleting the head of the list */
		ht->pListHead = p->pListNext;
	}
	if (p->pListNext != NULL) {
		p->pListNext->pListLast = p->pListLast;
	} else {
		ht->pListTail = p->pListLast;
	}
	if (ht->pInternalPointer == p) {
		ht->pInternalPointer = p->pListNext;
	}
	ht->nNumOfElements--;
	HANDLE_UNBLOCK_INTERRUPTIONS();

	if (ht->pDestructor) {
		ht->pDestructor(p->pData);
	}
	if (p->pData != &p->pDataPtr) {
		pefree(p->pData, ht->persistent);
	}
	retval = p->pListNext;
	pefree(p, ht->persistent);

	return retval;
}

void od_wrapper_hash_apply_with_argument(HashTable *ht, apply_func_arg_t apply_func, void *argument TSRMLS_DC)
{
	if(ht==NULL) return;

	Bucket *p;

	p = ht->pListHead;
	while (p != NULL) {
		int result = apply_func(p, argument TSRMLS_CC);

		if (result & ZEND_HASH_APPLY_REMOVE) {
			p = od_wrapper_hash_apply_deleter(ht, p);
		} else {
			p = p->pListNext;
		}
		if (result & ZEND_HASH_APPLY_STOP) {
			break;
		}
	}
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
	od_wrapper_object *intern = (od_wrapper_object*) object;
	od_wrapper_object *clone = NULL;

	clone = od_wrapper_object_new(intern->zo.ce TSRMLS_CC);

	*clone_ptr = clone;
}

void od_wrapper_object_dtor(void *object, zend_object_handle handle TSRMLS_DC)
{
	//FIXME
	//TODO
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

	efree(intern);
}

//FIXME
// need change the algorithm
int od_wrapper_count_elements_t(zval *object, long *count TSRMLS_DC)
{
	if(count==NULL) return FAILURE;

	*count = 1;

	od_wrapper_object* od_obj = (od_wrapper_object*)zend_object_store_get_object(object TSRMLS_CC);

	if(od_obj) {

		od_igbinary_unserialize_data local_igsd = od_obj->igsd;
		local_igsd.buffer_offset = 0;

		char* class_name;
		size_t class_name_len;

		od_igbinary_unserialize_chararray(&local_igsd, od_igbinary_get_type(&local_igsd), &class_name, &class_name_len);

		int member_num = od_igbinary_get_member_num(&local_igsd,od_igbinary_get_type(&local_igsd));

		int unset_num = 0;

		if(OD_HASH_NUM(od_obj->modified_properties)) {
			Bucket* p = od_obj->modified_properties->pListHead;

			while(p!=NULL) {

				if(p->pDataPtr == NULL) {
					unset_num ++;
				}

				p = p->pListNext;
			}
		}

		*count = member_num - unset_num;

		return SUCCESS;
	} else {
		return FAILURE;
	}
}

HashTable* od_wrapper_get_properties(zval *object TSRMLS_DC)
{
	od_wrapper_object* od_obj = (od_wrapper_object*)zend_object_store_get_object(object TSRMLS_CC);

	if(od_obj->all_get) return od_obj->zo.properties;

	//FIXME
	debug("@@@ DANGOURS!!!!!!!! in od_wrapper_get_properties for class '%s'",OD_CLASS_NAME(od_obj));

	if(!od_obj->initialized) od_wrapper_lazy_init(object,od_obj);

	debug_mem("start get all properties for class '%s'",OD_CLASS_NAME(od_obj));

	get_all_members(od_obj);

	// merge with default properties
	// FIXME
	// need deal with unset properties

	Bucket* p = od_obj->zo.ce->default_properties.pListHead;

	while(p!=NULL) {

		if(p->pData && !zend_hash_quick_exists(od_obj->zo.properties,p->arKey,p->nKeyLength,p->h)) {

			zend_hash_quick_add(od_obj->zo.properties,p->arKey,p->nKeyLength,p->h,p->pData,sizeof(zval*),NULL);

			(*((zval**)p->pData))->refcount ++;
		}

		p = p->pListNext;
	}

	debug_mem("end get all properties for class '%s'",OD_CLASS_NAME(od_obj));

	od_obj->all_get = 1;

	return od_obj->zo.properties;
}

zval *od_wrapper_read_property(zval *object, zval *member, int type TSRMLS_DC)
{
	zend_object *zobj;
	zval *tmp_member = NULL;
	zval **retval;
	zval *rv = NULL;
	zend_property_info *property_info;
	int silent;

	silent = (type == BP_VAR_IS);
	zobj = (zend_object*)zend_object_store_get_object(object);

	od_wrapper_object* od_obj = (od_wrapper_object*)zobj;

	if(!od_obj->initialized) od_wrapper_lazy_init(object,od_obj);

 	if (member->type != IS_STRING) {
 		ALLOC_ZVAL(tmp_member);
		*tmp_member = *member;
		INIT_PZVAL(tmp_member);
		zval_copy_ctor(tmp_member);
		convert_to_string(tmp_member);
		member = tmp_member;
	}

 	OD_CALLL_INFO;

#if DEBUG_OBJECT_HANDLERS
	fprintf(stderr, "Read object #%d property: %s\n", Z_OBJ_HANDLE_P(object), Z_STRVAL_P(member));
#endif

	/* make zend_get_property_info silent if we have getter - we may want to use it */
	property_info = zend_get_property_info(zobj->ce, member, (zobj->ce->__get != NULL) TSRMLS_CC);

	if (!property_info || zend_hash_quick_find(zobj->properties, property_info->name, property_info->name_length+1, property_info->h, (void **) &retval) == FAILURE) {

		zend_guard *guard;

		if (zobj->ce->__get &&
		    od_get_property_guard(zobj, property_info, member, &guard) == SUCCESS &&
		    !guard->in_get) {

			search_property(od_obj,property_info,&retval);

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
				if(IS_OD_WRAPPER(*retval)) {
					od_wrapper_add_obj_property((od_wrapper_object*)zobj, property_info, retval);
				} else if((*retval)->type == IS_ARRAY && array_contains_object(*retval)) {
					od_wrapper_add_modified_property(od_obj, property_info, retval);
				}
			}
		} else {

			if(!zobj->ce->__get) {
				search_property(od_obj,property_info,&retval);

				if(!retval)
				{
					retval = &EG(uninitialized_zval_ptr);
				} else {
					if(IS_OD_WRAPPER(*retval)) {
						od_wrapper_add_obj_property((od_wrapper_object*)zobj, property_info, retval);
					} else if((*retval)->type == IS_ARRAY && array_contains_object(*retval)) {
						od_wrapper_add_modified_property(od_obj, property_info, retval);
					}
				}
			}

			if (!silent && retval == &EG(uninitialized_zval_ptr)) {
					zend_error(E_NOTICE,"Undefined property: %s::$%s", zobj->ce->name, Z_STRVAL_P(member));
			}
		}
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

	zobj = (zend_object *)zend_object_store_get_object(object TSRMLS_CC);

	od_wrapper_object* od_obj = (od_wrapper_object*)zobj;

	if(!od_obj->initialized) od_wrapper_lazy_init(object,od_obj);

 	if (member->type != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
	}

 	OD_CALLL_INFO;

#if DEBUG_OBJECT_HANDLERS
	fprintf(stderr, "Ptr object #%d property: %s\n", Z_OBJ_HANDLE_P(object), Z_STRVAL_P(member));
#endif

	property_info = zend_get_property_info(zobj->ce, member, (zobj->ce->__get != NULL) TSRMLS_CC);

	if (!property_info || zend_hash_quick_find(zobj->properties, property_info->name, property_info->name_length+1, property_info->h, (void **) &retval) == FAILURE) {
		zval *new_zval;
		zend_guard *guard;

		if (!zobj->ce->__get ||
			od_get_property_guard(zobj, property_info, member, &guard) != SUCCESS ||
			(property_info && guard->in_get)) {

			if(!zobj->ce->__get) {
				search_property(od_obj,property_info,&retval);

				if(retval == NULL) {
					new_zval = &EG(uninitialized_zval);

					/* od_error(E_NOTICE, "Undefined property: %s", Z_STRVAL_P(member)); */
					new_zval->refcount++;
					zend_hash_quick_update(zobj->properties, property_info->name, property_info->name_length+1, property_info->h, &new_zval, sizeof(zval *), (void **) &retval);

					od_wrapper_add_modified_property(od_obj, property_info, retval);
				} else {
					if(IS_OD_WRAPPER(*retval)) {
						od_wrapper_add_obj_property(od_obj, property_info, retval);
					} else {
						od_wrapper_add_modified_property(od_obj, property_info, retval);
					}
				}
			} else {
				/* we don't have access controls - will just add it */
				new_zval = &EG(uninitialized_zval);

				/* od_error(E_NOTICE, "Undefined property: %s", Z_STRVAL_P(member)); */
				new_zval->refcount++;
				zend_hash_quick_update(zobj->properties, property_info->name, property_info->name_length+1, property_info->h, &new_zval, sizeof(zval *), (void **) &retval);

				od_wrapper_add_modified_property(od_obj, property_info, retval);
			}
		} else {
			/* we do have getter - fail and let it try again with usual get/set */
			retval = NULL;
		}

		//Will treat this
	} else {
		// Will treat this as modification to existed key
		if(IS_OD_WRAPPER(*retval)) {
			od_wrapper_add_obj_property(od_obj, property_info, retval);
		} else {
			od_wrapper_add_modified_property(od_obj, property_info, retval);
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

	zobj = (zend_object *)zend_object_store_get_object(object TSRMLS_CC);

	od_wrapper_object* od_obj = (od_wrapper_object*)zobj;

	if(!od_obj->initialized) od_wrapper_lazy_init(object,od_obj);

 	if (member->type != IS_STRING) {
 		ALLOC_ZVAL(tmp_member);
		*tmp_member = *member;
		INIT_PZVAL(tmp_member);
		zval_copy_ctor(tmp_member);
		convert_to_string(tmp_member);
		member = tmp_member;
	}

 	OD_CALLL_INFO;

	property_info = zend_get_property_info(zobj->ce, member, (zobj->ce->__set != NULL) TSRMLS_CC);

	if (property_info && zend_hash_quick_find(zobj->properties, property_info->name, property_info->name_length+1, property_info->h, (void **) &variable_ptr) == SUCCESS) {
		/* if we already have this value there, we don't actually need to do anything */
		if (*variable_ptr != value) {
			od_wrapper_modify_property(variable_ptr,value,property_info,od_obj);
		}
	} else {

		search_property(od_obj,property_info,&variable_ptr);

		if(variable_ptr!=NULL && *variable_ptr!=NULL) {
			od_wrapper_modify_property(variable_ptr,value,property_info,od_obj);
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
				zend_hash_quick_update(zobj->properties, property_info->name, property_info->name_length+1, property_info->h, &value, sizeof(zval *), (void **) &foo);

				od_wrapper_add_modified_property(od_obj, property_info, foo);
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

	zobj = (zend_object*)zend_object_store_get_object(object);

	od_wrapper_object* od_obj = (od_wrapper_object*)zobj;

	if(!od_obj->initialized) od_wrapper_lazy_init(object,od_obj);

 	if (member->type != IS_STRING) {
 		ALLOC_ZVAL(tmp_member);
		*tmp_member = *member;
		INIT_PZVAL(tmp_member);
		zval_copy_ctor(tmp_member);
		convert_to_string(tmp_member);
		member = tmp_member;
	}

 	OD_CALLL_INFO;

	property_info = zend_get_property_info(zobj->ce, member, (zobj->ce->__unset != NULL) TSRMLS_CC);

	if (!property_info || zend_hash_del(zobj->properties, property_info->name, property_info->name_length+1) == FAILURE) {

		zend_uchar success = 0;

		if(property_info) {
			zval** val;
			if(search_property(od_obj,property_info,&val) == SUCCESS) {
				success = (zend_hash_del(zobj->properties, property_info->name, property_info->name_length+1) == SUCCESS);
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
		} else {
			debug("unset key '%s'", property_info->name);
			od_wrapper_add_modified_property(od_obj, property_info, NULL);
		}
	} else {
		debug("unset key '%s'", property_info->name);
		od_wrapper_add_modified_property(od_obj, property_info, NULL);
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

	zobj = (zend_object *)zend_object_store_get_object(object TSRMLS_CC);

	od_wrapper_object* od_obj = (od_wrapper_object*)zobj;
	if(!od_obj->initialized) od_wrapper_lazy_init(object,od_obj);

	if (member->type != IS_STRING) {
		ALLOC_ZVAL(tmp_member);
		*tmp_member = *member;
		INIT_PZVAL(tmp_member);
		zval_copy_ctor(tmp_member);
		convert_to_string(tmp_member);
		member = tmp_member;
	}

	OD_CALLL_INFO;

#if DEBUG_OBJECT_HANDLERS
	fprintf(stderr, "Read object #%d property: %s\n", Z_OBJ_HANDLE_P(object), Z_STRVAL_P(member));
#endif

	property_info = zend_get_property_info(zobj->ce, member, 1 TSRMLS_CC);

	int searched = 0;

	if (!property_info || ( zend_hash_quick_find(zobj->properties, property_info->name, property_info->name_length+1, property_info->h, (void **) &value) == FAILURE && (searched=1, search_property(od_obj,property_info,&value)) == FAILURE)) {
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

		if(searched && value) {
			if(IS_OD_WRAPPER(*value)) {
				od_wrapper_add_obj_property((od_wrapper_object*)zobj, property_info, value);
			} else if((*value)->type == IS_ARRAY && array_contains_object(*value)) {
				od_wrapper_add_modified_property(od_obj, property_info, value);
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
	//FIXME
	// to be removed

	od_wrapper_object* od_obj = (od_wrapper_object*)zend_object_store_get_object(*object_ptr TSRMLS_CC);

	debug("$$$ in get_method, func name '%s' for class %s",method_name, OD_CLASS_NAME(od_obj));

	if(object_ptr && *object_ptr && strncmp("__wakeup",method_name,method_len)!=0) {
		od_wrapper_lazy_init(*object_ptr,NULL);
	}

	return std_object_handlers.get_method(object_ptr,method_name,method_len);
}

// Static Utilities Definition

inline void od_wrapper_lazy_init(zval* obj, od_wrapper_object* od_obj)
{
	assert(obj);

	if(od_obj==NULL) {
		od_obj = (od_wrapper_object*)zend_object_store_get_object(obj TSRMLS_CC);
	}

	if(od_obj->initialized) return;

	od_obj->initialized = 1;

	//XXX
	//no igsd->buffer_offset points to the first member

	od_obj->pos_info = NULL;
	od_obj->modified_properties = NULL;

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

zval* od_wrapper_unserialize(od_igbinary_unserialize_data *igsd, zend_uchar skip)
{
	zval* val=NULL;

	od_igbinary_type t = (od_igbinary_type) od_igbinary_get_type(igsd TSRMLS_CC);

	switch (t) {
		case od_igbinary_type_object8:
		case od_igbinary_type_object16:
		case od_igbinary_type_object32:
		{
			char* class_name;
			size_t class_name_len;

			size_t class_info_offset = igsd->buffer_offset-1;

			od_igbinary_unserialize_chararray(igsd, t, &class_name, &class_name_len);

			int member_num = od_igbinary_get_member_num(igsd,od_igbinary_get_type(igsd));

			int value_len = od_igbinary_get_value_len(igsd);

			if(skip) {
				igsd->buffer_offset += value_len;
				debug("skip %d bytes", value_len);
				break;
			}

			MAKE_STD_ZVAL(val);
			val = new_od_wrapper(val,igsd->buffer + class_info_offset, igsd->buffer_offset - class_info_offset + value_len, class_name, class_name_len, member_num, value_len, igsd->buffer_offset - class_info_offset);

			igsd->buffer_offset += value_len;
		}
			break;
		case od_igbinary_type_array8:
		case od_igbinary_type_array16:
		case od_igbinary_type_array32:
		{
			size_t i;

			char *key;
			size_t key_len = 0;
			long key_index = 0;

			od_igbinary_type key_type;

			HashTable *h;

			size_t n = od_igbinary_get_member_num(igsd,t);

			if(n<0) {
				od_error(E_ERROR,"array size could not be less than zero");
				break;
			}

			int value_len = od_igbinary_get_value_len(igsd);

			if(skip) {
				igsd->buffer_offset += value_len;
				debug("skip %d bytes", value_len);
				break;
			}

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

				zval* v = od_wrapper_unserialize(igsd,0);

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
			size_t tmp_size_t;

			if (od_igbinary_unserialize_chararray(igsd, t, &tmp_chararray, &tmp_size_t TSRMLS_CC)) {
				break;
			}

			if(!skip) {
				MAKE_STD_ZVAL(val);
				Z_TYPE_P(val) = IS_STRING;
				Z_STRVAL_P(val) = tmp_chararray;
				Z_STRLEN_P(val) = tmp_size_t;
				SET_OD_REFCOUNT(val);
			}
		}
			break;
		case od_igbinary_type_string_empty:
			if(!skip) {
				MAKE_STD_ZVAL(val);
				ZVAL_EMPTY_STRING(val);
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
			if(!skip) {
				MAKE_STD_ZVAL(val);
				ZVAL_LONG(val, tmp_long);
			}
		}
			break;
		case od_igbinary_type_null:
			if(!skip) {
				MAKE_STD_ZVAL(val);
				ZVAL_NULL(val);
			}
			break;
		case od_igbinary_type_bool_false:
			if(!skip) {
				MAKE_STD_ZVAL(val);
				ZVAL_BOOL(val, 0);
			}
			break;
		case od_igbinary_type_bool_true:
			if(!skip) {
				MAKE_STD_ZVAL(val);
				ZVAL_BOOL(val, 1);
			}
			break;
		case od_igbinary_type_double:
		{
			double tmp_double;
			if (od_igbinary_unserialize_double(igsd, t, &tmp_double TSRMLS_CC)) {
				break;
			}
			if(!skip) {
				MAKE_STD_ZVAL(val);
				ZVAL_DOUBLE(val, tmp_double);
			}
		}
			break;
		default:
			od_error(E_ERROR, "od_igbinary_unserialize_zval: unknown type '%02x', position %zu", t, igsd->buffer_offset);
			break;
	}

	if(val!=NULL)
	{
		val->refcount ++;
	}

	debug("data type: %2x skip: %s val: %s",t,skip?"yes":"no", val?"yes":"null");


	return val;
}

void search_member(od_wrapper_object* od_obj, zend_property_info *property_info, zval*** retval_p)
{
	char* member_name = property_info->name;
	int member_len = property_info->name_length;
	ulong hash = property_info->h;

	debug("in search_member for key %s",member_name);

	assert(retval_p);

	*retval_p = NULL;

	char* name = NULL;
	size_t len = 0;
	long index;

	zval* member = NULL;

	od_pos* val_pos;

	zend_uchar found=0;

	od_igbinary_unserialize_data* igsd = &od_obj->igsd;

	od_pos** member_pos = NULL;

	if(od_obj->pos_info && zend_hash_quick_find(od_obj->pos_info, member_name, member_len+1, hash, (void **) &member_pos) == SUCCESS) {

		debug("find pos info in pos_info for %s",member_name);
		if(member_pos!=NULL)
		{
			zval** tmp_val_p = NULL;

			if(od_obj->modified_properties && zend_hash_quick_find(od_obj->modified_properties, member_name, member_len+1, hash, (void **) &tmp_val_p) == SUCCESS && tmp_val_p && *tmp_val_p==NULL)
			{
				//this property is unset
				*retval_p = NULL;
			}else{
				size_t current_offset = igsd->buffer_offset;

				igsd->buffer_offset = (*member_pos)->key_start - igsd->buffer;

				od_igbinary_unserialize_skip_key(igsd);

				member = od_wrapper_unserialize(igsd,0);

				zend_hash_quick_add(od_obj->zo.properties, member_name, member_len+1, hash, &member, sizeof(zval*), (void**)retval_p);

				igsd->buffer_offset = current_offset;
			}
		}

		return;
	}

	while(OD_NOT_END(od_obj))
	{
		OD_MAKE_POS(val_pos);

		val_pos->key_start = OD_OFFSET_POS(od_obj);

		od_igbinary_unserialize_get_key(igsd, &name, &len, &index);

		if(name==NULL) {
			od_error(E_ERROR, "key for object could not be null");
		}

		found = (member_len == len && strncmp(member_name,name,len)==0);

		member = od_wrapper_unserialize(igsd,!found);

		val_pos->value_end = OD_OFFSET_POS(od_obj);

		ulong h = zend_get_hash_value(name, len+1);

		OD_HASH_LAZY_INIT(od_obj->pos_info);
		zend_hash_quick_add(od_obj->pos_info, name, len+1, h, &val_pos, sizeof(od_pos*), (NULL));

		if(member!=NULL) {
			zend_hash_quick_add(od_obj->zo.properties, name, len+1, h, &member, sizeof(zval*), (void**)retval_p);
		}

		//FIXME
		// to be removed
		// DEBUG start
		char tname[256];
		memcpy(tname,name,len);
		tname[len]='\0';

		int t;
		for(t=0;t<len;t++) {
			if(tname[t]=='\0') {
				tname[t]='0';
			}
		}
		char c=tname[len];
		tname[len]='\0';
		if(member!=NULL) {
			debug("add property '%s' for class %s member type: %d",tname, od_obj->zo.ce->name, member->type);
		}
		debug("add pos info for key %s", tname);
		tname[len]=c;
		// DEBUG end

		if(found){
			break;
		}
	}

	if(!found) {

		*retval_p = NULL;

		debug("will search in default properties for member '%s' with len '%d' hash: %u",member_name, member_len, hash);

		if(zend_hash_quick_find(&od_obj->zo.ce->default_properties,member_name, member_len+1, hash,(void**)retval_p)==SUCCESS) {
			debug("found in default property");
			if(*retval_p) {
				zend_hash_quick_add(od_obj->zo.properties, member_name, member_len+1, hash, *retval_p, sizeof(zval*), (void**)retval_p);
				(**retval_p)->refcount ++;

				od_wrapper_add_modified_property(od_obj, property_info, *retval_p);
			}
		}
	}
}

int search_property(od_wrapper_object* od_obj,zend_property_info *property_info, zval*** retval_p)
{
	if(retval_p) {
		*retval_p = NULL;
	}

	if(od_obj && property_info && retval_p)
	{
		debug("search in blob data for member '%s' with len %d and hash %u",property_info->name,property_info->name_length,property_info->h);
		search_member(od_obj,property_info, retval_p);
	}

	return (retval_p && *retval_p)? SUCCESS:FAILURE;
}

//FIXME
// to be fixed
void get_all_members(od_wrapper_object* od_obj)
{
	char* name = NULL;
	size_t len = 0;
	long index;

	zval* member = NULL;

	od_pos* val_pos;

	od_igbinary_unserialize_data* igsd = &od_obj->igsd;

	if(od_obj->pos_info) {

		od_pos** member_pos = NULL;

		Bucket* p = od_obj->pos_info->pListHead;

		while(p!=NULL) {

			member_pos = (od_pos**)p->pData;

			if(member_pos!=NULL)
			{
				if(!zend_hash_quick_exists(od_obj->zo.properties, p->arKey, p->nKeyLength, p->h)) {

					zval** tmp_val_p = NULL;

					if(!(od_obj->modified_properties && zend_hash_quick_find(od_obj->modified_properties, p->arKey, p->nKeyLength, p->h, (void **) &tmp_val_p) == SUCCESS && tmp_val_p && *tmp_val_p==NULL))
					{
						size_t current_offset = igsd->buffer_offset;

						igsd->buffer_offset = (*member_pos)->key_start - igsd->buffer;

						od_igbinary_unserialize_skip_key(igsd);

						member = od_wrapper_unserialize(igsd,0);

						debug("add val from pos info for key '%s' with len %d", p->arKey, p->nKeyLength);

						zend_hash_quick_add(od_obj->zo.properties, p->arKey, p->nKeyLength, p->h, &member, sizeof(zval*), NULL);

						igsd->buffer_offset = current_offset;
					}

				}
			}


			p = p->pListNext;
		}
	}

	while(OD_NOT_END(od_obj))
	{
		OD_MAKE_POS(val_pos);

		val_pos->key_start = OD_OFFSET_POS(od_obj);

		od_igbinary_unserialize_get_key(igsd, &name, &len, &index);

		if(name==NULL) {
			od_error(E_ERROR, "key for object could not be null");
		}

		member = od_wrapper_unserialize(igsd,0);

		val_pos->value_end = OD_OFFSET_POS(od_obj);

		member->refcount++;

		ulong h = zend_get_hash_value(name, len+1);

		zend_hash_quick_add(od_obj->zo.properties, name, len+1, h, &member, sizeof(zval*), NULL);

		OD_HASH_LAZY_INIT(od_obj->pos_info);
		zend_hash_quick_add(od_obj->pos_info, name, len+1, h, &val_pos, sizeof(od_pos*), (NULL));

		//FIXME
		// to be removed
		// DEBUG start
		char tname[256];
		memcpy(tname,name,len);
		tname[len]='\0';

		int t;
		for(t=0;t<len;t++) {
			if(tname[t]=='\0') {
				tname[t]='0';
			}
		}
		char c=tname[len];
		tname[len]='\0';
		debug("add property '%s' for class %s",tname, od_obj->zo.ce->name);
		tname[len]=c;
		// DEBUG end
	}
}

int od_wrapper_move_obj(void* bkt, void* arg)
{
	assert(arg);

	if(!bkt || !arg) {
		return ZEND_HASH_APPLY_REMOVE;
	}

	Bucket* p = (Bucket*)bkt;

	zval* s_obj = *(zval**)p->pDataPtr;

	if(s_obj->type != IS_OBJECT) {
		return ZEND_HASH_APPLY_REMOVE;
	}

	if(check_od_wrapper_ob_status(s_obj)) {

		HashTable** p_ht = (HashTable**)arg;

		od_wrapper_add_property_ex(p_ht, p->arKey, p->nKeyLength -1, p->h, (zval**)p->pDataPtr);

		debug("xxx will move key: '%s' len: %d h: %u from obj list to modified list",p->arKey, p->nKeyLength -1, p->h);

		return ZEND_HASH_APPLY_REMOVE;
	} else {
		return ZEND_HASH_APPLY_KEEP;
	}
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

int check_od_wrapper_ob_status(zval* obj)
{
	if(!obj) return 0;

	od_wrapper_object* od_obj = (od_wrapper_object*)zend_object_store_get_object(obj TSRMLS_CC);

	if(OD_HASH_NUM(od_obj->obj_properties) == 0) {
		return OD_HASH_NUM(od_obj->modified_properties) != 0;
	} else {

		//FIXME
		/*
		debug("obj_properties: before");
		print_ht(od_obj->obj_properties);

		debug("modified_properties: before");
		print_ht(od_obj->modified_properties);
		*/

		od_wrapper_hash_apply_with_argument(od_obj->obj_properties,od_wrapper_move_obj,&od_obj->modified_properties);

		/*
		debug("obj_properties: after");
		print_ht(od_obj->obj_properties);

		debug("modified_properties: after");
		print_ht(od_obj->modified_properties);
		*/

		return OD_HASH_NUM(od_obj->modified_properties) != 0;
	}
}

inline int od_wrapper_exists_property(HashTable* ht, zend_property_info *property_info)
{
	return ht?zend_hash_quick_exists(ht, property_info->name, property_info->name_length+1, property_info->h):0;
}

inline void od_wrapper_del_property(HashTable* ht, zend_property_info *property_info)
{
	if(ht) {
		zend_hash_del_key_or_index(ht,property_info->name, property_info->name_length+1, property_info->h, HASH_DEL_INDEX);
	}
}

inline void od_wrapper_add_property_ex(HashTable** p_ht, char* name, int len, ulong h, zval** value)
{
	if(value && *value) {
		(*value)->refcount++;
	}

	OD_HASH_LAZY_INIT(*p_ht);
	zend_hash_quick_update(*p_ht, name, len+1, h, &value, sizeof(zval **), NULL);
}

inline void od_wrapper_add_property(HashTable** p_ht, zend_property_info *property_info, zval** value)
{
	od_wrapper_add_property_ex(p_ht, property_info->name, property_info->name_length, property_info->h, value);
}

inline void od_wrapper_add_modified_property(od_wrapper_object* od_obj, zend_property_info *property_info, zval** value)
{
	debug("add modified for class %s: key %s len:%d h:%u",OD_CLASS_NAME(od_obj),property_info->name,property_info->h,property_info->name_length);

	od_wrapper_add_property(&od_obj->modified_properties, property_info, value);
}

inline void od_wrapper_add_obj_property(od_wrapper_object* od_obj, zend_property_info *property_info, zval** value)
{
	if(!od_wrapper_exists_property(od_obj->modified_properties, property_info)) {
		debug("add obj key %s h:%u len:%d ",property_info->name,property_info->h,property_info->name_length);

		od_wrapper_add_property(&od_obj->obj_properties, property_info, value);
	}
}

inline void od_wrapper_del_obj_property(od_wrapper_object* od_obj, zend_property_info *property_info)
{
	od_wrapper_del_property(od_obj->obj_properties, property_info);
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

	od_wrapper_add_modified_property(od_obj,property_info,variable_ptr);

	if((*variable_ptr)->type == IS_OBJECT) {
		od_wrapper_del_obj_property(od_obj,property_info);
	}
}
