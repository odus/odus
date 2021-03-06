/*
  +----------------------------------------------------------------------+
  | See COPYING file for further copyright information                   |
  +----------------------------------------------------------------------+
  | Author: Pai Deng <pdeng@zynga.com>                                   |
  | See CREDITS for contributors                                         |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_odus.h"

#include "od_def.h"
#include "od_hash.h"
#include "odwrapper.h"
#include "ext/standard/php_incomplete_class.h"

#include "ext/standard/php_var.h"
#include "ext/standard/php_smart_str.h"


ZEND_DECLARE_MODULE_GLOBALS(odus)

static void php_odus_init_globals(zend_odus_globals* odus_globals TSRMLS_DC)
{
	odus_globals->remove_default = 0;
	odus_globals->od_throw_exceptions = 0;
	odus_globals->od_reduce_fatals = 0;
	odus_globals->format_version = 2;
	odus_globals->force_release_memory = 1;
	odus_globals->static_strings_file = OD_IGBINARY_DEFAULT_STATIC_STRINGS_FILE;
}

/* {{{ odus_functions[]
 *
 * Every user visible function must have an entry in odus_functions[].
 */
zend_function_entry odus_functions[] = {
	PHP_FE(od_serialize,	NULL)
	PHP_FE(od_unserialize,	NULL)
	PHP_FE(od_version,	NULL)
	PHP_FE(od_format_version,	NULL)
	PHP_FE(od_format_match,	NULL)
	PHP_FE(od_overwrite_function,	NULL)
	PHP_FE(od_refresh_odwrapper,	NULL)
	PHP_FE(od_getobjectkeys_without_key, NULL)
	PHP_FE(od_get_mangled_name, NULL)
	PHP_FE(od_release_memory, NULL)
	PHP_FE(od_reserialize, NULL)
	PHP_FE(od_is_wrapper,	NULL)
	{NULL, NULL, NULL}	/* Must be the last line in odus_functions[] */
};
/* }}} */

/* {{{ odus_module_entry
 */
zend_module_entry odus_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"odus",
	odus_functions,
	PHP_MINIT(odus),
	PHP_MSHUTDOWN(odus),
	NULL,
	NULL,
	PHP_MINFO(odus),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_ODUS
ZEND_GET_MODULE(odus)
#endif

static void apply_sleep_array(od_wrapper_object* od_obj, HashTable* h);

void normal_od_wrapper_serialize(od_igbinary_serialize_data* igsd, zval* obj, uint8_t is_root);

extern zend_object_handlers od_wrapper_object_handlers;

extern uint8_t is_od_wrapper_obj_modified(od_wrapper_object* od_obj,uint8_t has_sleep, int* member_num_diff, struct hash_si * visited_od_wrappers);

extern void search_member(od_wrapper_object* od_obj, const char* member_name, uint32_t member_len, uint32_t hash, ODBucket** ret_bkt, member_pos* ret_pos);

extern int od_wrapper_skip_value(od_igbinary_unserialize_data *igsd);

void od_overwrite_function(char* old, char* new) {
	if(EG(function_table)==NULL || old==NULL || new==NULL) return;

	zend_function* func = NULL;

	if(zend_hash_find(EG(function_table),new,strlen(new)+1,(void**)&func)==SUCCESS) {

		if(func)
		{
			zend_hash_update(EG(function_table),old,strlen(old)+1,func,sizeof(zend_function),NULL);
		}
	}
}

PHP_INI_BEGIN()
    STD_PHP_INI_BOOLEAN("odus.remove_default",      "0",    PHP_INI_SYSTEM, OnUpdateBool,              remove_default,         zend_odus_globals, odus_globals)
    STD_PHP_INI_BOOLEAN("odus.throw_exceptions",      "0",    PHP_INI_SYSTEM, OnUpdateBool,              od_throw_exceptions,         zend_odus_globals, odus_globals)
    STD_PHP_INI_BOOLEAN("odus.reduce_fatals",      "0",    PHP_INI_SYSTEM, OnUpdateBool,              od_reduce_fatals,         zend_odus_globals, odus_globals)
    STD_PHP_INI_BOOLEAN("odus.format_version",    "2",    PHP_INI_SYSTEM, OnUpdateLong,              format_version,          zend_odus_globals, odus_globals)
    STD_PHP_INI_BOOLEAN("odus.force_release_memory",      "0",    PHP_INI_SYSTEM, OnUpdateBool,              force_release_memory,         zend_odus_globals, odus_globals)
    STD_PHP_INI_BOOLEAN("odus.static_strings_file",    OD_IGBINARY_DEFAULT_STATIC_STRINGS_FILE,    PHP_INI_SYSTEM, OnUpdateString,              static_strings_file,          zend_odus_globals, odus_globals)
PHP_INI_END()

zend_class_entry *odus_exception_ce;

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(odus)
{
	zend_class_entry ce;

	ZEND_INIT_MODULE_GLOBALS(odus, php_odus_init_globals, NULL);

	/* Odus exception class */
	INIT_CLASS_ENTRY(ce, "OdusException", NULL);
	odus_exception_ce = zend_register_internal_class_ex(&ce, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
	odus_exception_ce->ce_flags |= ZEND_ACC_FINAL;

	REGISTER_INI_ENTRIES();

	od_wrapper_init(TSRMLS_C);

	if (od_igbinary_init(TSRMLS_C) != 0) {
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(odus)
{
	UNREGISTER_INI_ENTRIES();

	od_wrapper_shutdown(TSRMLS_C);

	od_igbinary_shutdown(TSRMLS_C);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(odus)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "odus support", "enabled");
	php_info_print_table_row(2, "odus version", OD_VERSION);
	php_info_print_table_row(2, "odus static strings file", ODUS_G(static_strings_file));

#ifdef ODDEBUG
	php_info_print_table_row(2, "odus type", "debug version");
#else
	php_info_print_table_row(2, "odus type", "release version");
#endif

	php_info_print_table_row(2, "odus class", PHP_ODUS_OBJECT_NAME);

	int i=0;

	while(odus_functions[i].fname!=NULL) {
		php_info_print_table_row(2, "odus function", odus_functions[i].fname);
		i++;
	}
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

inline static void deal_with_unmodified_object(od_igbinary_serialize_data* igsd, od_wrapper_object* od_obj, uint8_t is_root) {
	od_igbinary_serialize_memcpy(igsd,od_obj->igsd.buffer,od_obj->igsd.buffer_size);
}

static uint8_t check_sleep(zval* obj, od_wrapper_object* od_obj) {

	uint8_t has_sleep = 0;

	if(obj && od_obj && od_obj->zo.ce && od_obj->zo.ce != PHP_IC_ENTRY && zend_hash_exists(&od_obj->zo.ce->function_table, "__sleep", sizeof("__sleep"))) {

		/* function name string */
		zval f;
		zval* h = NULL;
		HashTable* flip_h = NULL;

		INIT_PZVAL(&f);
		ZVAL_STRINGL(&f, "__sleep", sizeof("__sleep") - 1, 0);

		if (call_user_function_ex(CG(function_table), &obj, &f, &h, 0, 0, 1, NULL TSRMLS_CC) == SUCCESS && !EG(exception)) {

			if (h) {
				if (Z_TYPE_P(h) == IS_ARRAY) {

					has_sleep = 1;

					apply_sleep_array(od_obj,Z_ARRVAL_P(h));
				} else {
					od_error(E_NOTICE, "__sleep should return an array only "
							"containing the names of instance-variables to "
							"serialize");
				}
			}
		}

		/* cleanup */
		if (h) {
			zval_ptr_dtor(&h);
		}
	}

	return has_sleep;
}

static void apply_sleep_array(od_wrapper_object* od_obj, HashTable* h) {

	if(!h || !od_obj) return;

	Bucket* p = h->pListHead;

	zval* data = NULL;

	ODBucket* bkt = NULL;

	ulong p_hash;
	uint32_t o_hash;

	zend_property_info *property_info = NULL;

	while(p!=NULL) {

		data = (zval*) p->pDataPtr;

		if(data) {
			if(data->type != IS_STRING) {
				od_error(E_ERROR,"item of sleep array must be string type");
			} else {

				p_hash = zend_get_hash_value(Z_STRVAL_P(data),Z_STRLEN_P(data)+1);
				o_hash = OD_HASH_VALUE(p_hash);

				if(od_hash_find(od_obj->od_properties,Z_STRVAL_P(data),Z_STRLEN_P(data),o_hash,&bkt)==SUCCESS) {
					if(bkt) {
						OD_SET_SLEEP(*bkt);

					}
				} else {

					if(zend_hash_quick_find(&(od_obj->zo.ce->properties_info),Z_STRVAL_P(data),Z_STRLEN_P(data)+1, p_hash, (void **) &property_info)==SUCCESS) {
						if(od_hash_find(od_obj->od_properties,property_info->name,property_info->name_length,OD_HASH_VALUE(property_info->h),&bkt)==SUCCESS) {
							if(bkt) {
								OD_SET_SLEEP(*bkt);

							}
						}
					}
				}
			}
		}

		p = p->pListNext;
	}
}

static void deal_with_new_properties(od_wrapper_object* od_obj, od_igbinary_serialize_data* igsd, od_igbinary_unserialize_data* local_igsd, uint8_t has_sleep) {

	uint32_t i;
	ODHashTable* ht = od_obj->od_properties;
	ODBucket* bkt = NULL;

	for(i=0;i<ht->size;i++) {
		bkt=ht->buckets+i;

		if(OD_IS_OCCUPIED(*bkt) && OD_IS_NEW(*bkt) && bkt->data!=NULL && (!has_sleep || OD_IS_SLEEP(*bkt)) && !(ODUS_G(remove_default) && !OD_IS_MODIFIED(*bkt) && OD_IS_DEFAULT(*bkt))) {
			uint32_t start_pos = igsd->buffer_size;
			debug("key %s is new for class %s", bkt->key?bkt->key:"null", OD_CLASS_NAME(od_obj));

			// A new property requires updating string table always.
			if(igsd->compact_strings && !igsd->string_table_update) {
				igsd->string_table_update = true;
				od_igbinary_clone_string_table(igsd, local_igsd TSRMLS_CC);
			}

			if(bkt->key_len == 0) {
				od_igbinary_serialize_long(igsd, bkt->hash);
			} else {
				od_igbinary_serialize_string(igsd, bkt->key, bkt->key_len);
			}

			zval *z = (zval*)bkt->data;
			normal_od_wrapper_serialize(igsd, (zval*)bkt->data,0);
		}
	}
}

static void deal_with_modified_properties(od_wrapper_object* od_obj, od_igbinary_serialize_data* igsd, od_igbinary_unserialize_data* local_igsd, uint8_t has_sleep) {

	uint32_t i;
	ODHashTable* ht = od_obj->od_properties;
	ODBucket* bkt = NULL;

	uint32_t modified_num = 0;

	for(i=0;i<ht->size;i++) {
		bkt=ht->buckets+i;

		if(OD_IS_OCCUPIED(*bkt) && !OD_IS_NEW(*bkt) && OD_IS_MODIFIED(*bkt)) {

			debug("key %s is modified for class %s", bkt->key?bkt->key:"null", OD_CLASS_NAME(od_obj));

			modified_num ++ ;
		}
	}

	if(modified_num>0) {
		uint32_t idx=0;

		member_pos* pos_info = (member_pos*)emalloc(modified_num*sizeof(member_pos));

		if(pos_info) {
			for(i=0;i<ht->size;i++) {
				bkt=ht->buckets+i;

				if(OD_IS_OCCUPIED(*bkt) && !OD_IS_NEW(*bkt) && OD_IS_MODIFIED(*bkt)) {
					search_member(od_obj,bkt->key,bkt->key_len,bkt->hash,NULL,pos_info+idx);

					if(has_sleep && !OD_IS_SLEEP(*bkt)){
						pos_info[idx].data = NULL;
					} else {
						pos_info[idx].data = (zval*)bkt->data;
					}

					idx++;
				}
			}

			//use select sort now because it is faster than quick sort when the number of items is small
			uint32_t k,j;

			member_pos tmp_pos;

			for(i=0;i<modified_num;i++) {
				k=i;
				for(j=i+1;j<modified_num;j++) {
					if(pos_info[j].key_start_offset < pos_info[k].key_start_offset){
						k=j;
					}
				}

				if(k!=i){
					tmp_pos = pos_info[k];
					pos_info[k] = pos_info[i];
					pos_info[i] = tmp_pos;
				}
			}

			//now serialize these modified properties
			for(i=0;i<modified_num;i++) {
				if(pos_info[i].data==NULL) {
					od_igbinary_serialize_memcpy(igsd, OD_LOCAL_OFFSET_POS(*local_igsd), pos_info[i].key_start_offset - local_igsd->buffer_offset);
				} else {

					od_igbinary_serialize_memcpy(igsd, OD_LOCAL_OFFSET_POS(*local_igsd), pos_info[i].val_start_offset - local_igsd->buffer_offset);

					//serialize value
					uint32_t value_start = igsd->buffer_size;

					if(igsd->compact_strings && !igsd->string_table_update && (pos_info[i].data->type == IS_OBJECT || pos_info[i].data->type == IS_ARRAY) ) {
						igsd->string_table_update = true;
						od_igbinary_clone_string_table(igsd, local_igsd TSRMLS_CC);
					}
					normal_od_wrapper_serialize(igsd,pos_info[i].data,0);

					int32_t value_len = igsd->buffer_size - value_start;
				}

				local_igsd->buffer_offset = pos_info[i].val_end_offset;
			}

			efree(pos_info);
			pos_info = NULL;
		}
	}

	od_igbinary_serialize_memcpy(igsd, OD_LOCAL_OFFSET_POS(*local_igsd), local_igsd->buffer_size - local_igsd->buffer_offset);
}

static void normal_complete_serialize(od_igbinary_serialize_data* igsd, zval* obj) {
	if(obj->type == IS_ARRAY) {
		od_igbinary_serialize_array(igsd,obj,NULL,0,0,1);
	} else {
		od_igbinary_serialize_zval(igsd,obj);
	}
}

void normal_od_wrapper_serialize(od_igbinary_serialize_data* igsd, zval* obj, uint8_t is_root) {
	if(obj==NULL) {
		od_error(E_ERROR, "obj could not be NULL here");
		return;
	}

	debug("in %s",__FUNCTION__);

	od_wrapper_object* od_obj = NULL;

	if(IS_OD_WRAPPER(obj)) {
		od_obj=(od_wrapper_object*)zend_object_store_get_object(obj);
	}

	if(!od_obj) {
		// no od_obj means that is_root is false, so igsd has been initialized
		normal_complete_serialize(igsd, obj);
	}else{

		uint8_t modified = 0;
		int num_diff = 0;

		struct hash_si visited_od_wrappers;

		od_igbinary_unserialize_data local_igsd = od_obj->igsd;
		local_igsd.buffer_offset = 0;

		if(is_root) {
			igsd->root_id = local_igsd.root_id;
		}

		do {
			if (igsd->root_id != 0 && igsd->root_id != local_igsd.root_id) {
				// obj comes from a different root object, do full serializing to avoid string table corrupt.
				// eg. $user->currentEnv = $world->env, strings in $world->env refer to $world's string table,
				// On serializing $user we must convert them to refer to $user's string table, by full serializing,
				normal_complete_serialize(igsd, obj);
				break;
			}

			hash_si_init(&visited_od_wrappers, 16);
			modified = is_od_wrapper_obj_modified(od_obj, 0, &num_diff, &visited_od_wrappers);
			hash_si_deinit(&visited_od_wrappers);

			debug("od_serialize: class '%s' is %s modified",OD_CLASS_NAME(od_obj),modified?"":"not");

			if(!modified){
				deal_with_unmodified_object(igsd, od_obj, is_root);
				break;
			}else{

				uint8_t has_sleep = check_sleep(obj, od_obj);

				if(has_sleep) {
					hash_si_init(&visited_od_wrappers, 16);
					modified = is_od_wrapper_obj_modified(od_obj,1,&num_diff, &visited_od_wrappers);
					hash_si_deinit(&visited_od_wrappers);

					if(!modified) {
						deal_with_unmodified_object(igsd, od_obj, is_root);
						break;
					}
				}

				//TODO
				//will reduce default properties here
				//There are multiple choices for removing default properties:
				// normal
				// medium
				// agressive
				//There is time and storage tradeoff
				// removing more default properties we can save more storage but need more cpu time

				// removing default properties is optional

				debug("deal with changed part of object %s, num_diff: %d",OD_CLASS_NAME(od_obj), num_diff);

				char* class_name;
				uint32_t class_name_len;

				od_igbinary_unserialize_class_name(&local_igsd, od_igbinary_get_type(&local_igsd), &class_name, &class_name_len);

				//XXX
				//this position will not changed
				uint32_t local_array_info_offset = local_igsd.buffer_offset;

				int member_num = od_igbinary_get_member_num(&local_igsd,od_igbinary_get_type(&local_igsd));

				//XXX
				//this position may be changed latter
				uint32_t local_len_info_offset = local_igsd.buffer_offset;

				od_igbinary_skip_value_len(&local_igsd);

				if(num_diff !=0) {

					debug("diff num: %d",num_diff);

					int new_member_num = member_num + num_diff;

					od_igbinary_serialize_memcpy(igsd,local_igsd.buffer, local_array_info_offset);

					debug("new meber num is %d", new_member_num);

					assert(new_member_num>=0);

					if (new_member_num <= 0xff) {
						od_igbinary_serialize8(igsd, od_igbinary_type_array8 TSRMLS_CC);
						od_igbinary_serialize8(igsd, new_member_num TSRMLS_CC);
					} else if (new_member_num <= 0xffff) {
						od_igbinary_serialize8(igsd, od_igbinary_type_array16 TSRMLS_CC);
						od_igbinary_serialize16(igsd, new_member_num TSRMLS_CC);
					} else {
						od_igbinary_serialize8(igsd, od_igbinary_type_array32 TSRMLS_CC);
						od_igbinary_serialize32(igsd, new_member_num TSRMLS_CC);
					}
				} else {
					od_igbinary_serialize_memcpy(igsd,local_igsd.buffer, local_len_info_offset);
				}

				uint32_t g_len_info_pos = igsd->buffer_size;

				od_igbinary_serialize_skip_n(igsd,OD_IGBINARY_VALUE_LEN_SIZE);

				debug("deal with modified properties for class %s",OD_CLASS_NAME(od_obj));

				deal_with_modified_properties(od_obj, igsd, &local_igsd, has_sleep);

				debug("deal with new properties for class %s",OD_CLASS_NAME(od_obj));

				deal_with_new_properties(od_obj, igsd, &local_igsd, has_sleep);

				//modify value len
				od_igbinary_serialize_value_len(igsd, igsd->buffer_size - g_len_info_pos - OD_IGBINARY_VALUE_LEN_SIZE, g_len_info_pos);	
			}
		} while (0);

		if (is_root && igsd->compact_strings) {
			od_igbinary_serialize_update_string_table(igsd, &local_igsd TSRMLS_CC);
		}
	}
}

/* Return true if migration is needed, false otherwise. */
static bool check_need_migration(zval* obj) {
	uint32_t format_version = (uint32_t)ODUS_G(format_version);
	uint32_t header = -1;

	if(!IS_OD_WRAPPER(obj)) {
		od_error(E_ERROR, "check_need_migration: obj must be ODWrapper!");
		return true;
	}

	od_wrapper_object* od_obj = (od_wrapper_object*)zend_object_store_get_object(obj);
	od_igbinary_unserialize_data local_igsd = od_obj->igsd;

	local_igsd.buffer = local_igsd.original_buffer;
	local_igsd.buffer_offset = 0;

	od_igbinary_unserialize_header(&local_igsd, &header TSRMLS_CC);

	if ((header & OD_IGBINARY_FORMAT_VERSION_MASK) != format_version) {
		return true;
	}
	return false;
}

/* {{{ proto string od_serialize(object ODWrapper)
   serialize method only for ODWrapper */
PHP_FUNCTION(od_serialize)
{
	zval* z = NULL;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",&z)) {
		return;
	}

	Z_TYPE_P(return_value) = IS_STRING;
	Z_STRVAL_P(return_value) = NULL;
	Z_STRLEN_P(return_value) = 0;

	od_igbinary_serialize_data igsd = {0};

	if (od_igbinary_serialize_data_init(&igsd, Z_TYPE_P(z) != IS_OBJECT && Z_TYPE_P(z) != IS_ARRAY TSRMLS_CC)) {
		od_error(E_ERROR, "od_serialize: cannot init igsd");
		RETURN_NULL();
	}

	if (od_igbinary_serialize_header(&igsd TSRMLS_CC) != 0) {
		od_error(E_ERROR, "od_serialize: cannot write header");
		od_igbinary_serialize_data_deinit(&igsd TSRMLS_CC);
		RETURN_NULL();
	}

	if(IS_OD_WRAPPER(z) && !check_need_migration(z)) {
		debug("in od_serialize => file: %s function: %s line: %d",OD_FILE,OD_FUNCTION,OD_LINE);

		normal_od_wrapper_serialize(&igsd,z,1);
	} else {
		if (od_igbinary_serialize_zval(&igsd, z TSRMLS_CC) != 0) {
			od_igbinary_serialize_data_deinit(&igsd TSRMLS_CC);
			RETURN_NULL();
		}

		if (igsd.compact_strings) {
			if (od_igbinary_serialize_string_table(&igsd TSRMLS_CC) != 0) {
				od_igbinary_serialize_data_deinit(&igsd TSRMLS_CC);
				RETURN_NULL();
			}
		}
	}

	if (igsd.buffer) {
		Z_STRVAL_P(return_value) = (char*)igsd.buffer;
		Z_STRLEN_P(return_value) = igsd.buffer_size;

		od_igbinary_serialize_append_zero(&igsd);

		igsd.buffer = NULL;

		od_igbinary_serialize_data_deinit(&igsd TSRMLS_CC);
		return;
	} else {
		od_igbinary_serialize_data_deinit(&igsd TSRMLS_CC);
		RETURN_NULL();
	}
}

PHP_FUNCTION(od_overwrite_function)
{
	zval* old = NULL;
	zval* new = NULL;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz",&old,&new)) {
		return;
	}

	if(old == NULL || old->type != IS_STRING || !Z_STRVAL_P(old) || new == NULL || new->type != IS_STRING || !Z_STRVAL_P(new)) {
		od_error(E_ERROR,"usage: od_overwrite_function old_name new_name");
		return;
	}

	od_overwrite_function(Z_STRVAL_P(old),Z_STRVAL_P(new));

	Z_TYPE_P(return_value) = IS_NULL;
}

PHP_FUNCTION(od_version)
{
	Z_TYPE_P(return_value) = IS_STRING;
	Z_STRVAL_P(return_value) = estrndup(OD_VERSION, sizeof(OD_VERSION)-1);
	Z_STRLEN_P(return_value) = sizeof(OD_VERSION) -1;
}

PHP_FUNCTION(od_format_version)
{
	char* version = TEXT(ODUS_G(format_version));
	int len = sizeof(TEXT(ODUS_G(format_version))) -1;

	Z_TYPE_P(return_value) = IS_STRING;
	Z_STRVAL_P(return_value) = estrndup(version, len);
	Z_STRLEN_P(return_value) = len;
}

PHP_FUNCTION(od_is_wrapper)
{
	zval* z = NULL;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",&z)) {
		RETURN_BOOL(0);
	}
	if(IS_OD_WRAPPER(z)) {
		RETURN_BOOL(1);
	} else {
		RETURN_BOOL(0);
	}
}

PHP_FUNCTION(od_format_match)
{
	char *str;
	int len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &str, &len) == FAILURE) {
		RETURN_BOOL(0);
	}

	if (!str || len < OD_IGBINARY_VERSION_BYTES) {
		RETURN_BOOL(0);
	}

	int i, j;
	bool match;

	uint8_t val;

	ulong versions[] = {
		OD_IGBINARY_FORMAT_FLAG | OD_IGBINARY_FORMAT_VERSION_01,
		OD_IGBINARY_FORMAT_FLAG | OD_IGBINARY_FORMAT_VERSION_02,
	};

	for (j = 0; j < sizeof(versions) / sizeof(ulong); j ++) {
		ulong version = versions[j];

		match = true;
		for(i=OD_IGBINARY_VERSION_BYTES-1;i>=0;i--) {
			val = (version & 0xFF);

			if(val != (uint8_t)str[i]) {
				match = false;
				break;
			}

			version>>=8;
		}
		if (match) {
			RETURN_BOOL(1);
		}
	}

	RETURN_BOOL(0);
}

PHP_FUNCTION(od_refresh_odwrapper)
{
	zval* refresh = NULL;
	zval* replace = NULL;
	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz",&refresh,&replace)) {
		RETURN_BOOL(0);
	}

	if(refresh == NULL || Z_TYPE_P(refresh) != IS_OBJECT || !IS_OD_WRAPPER(refresh) || replace == NULL || Z_TYPE_P(replace) != IS_OBJECT || !IS_OD_WRAPPER(replace)) {
		RETURN_BOOL(0);
	}

	zend_object *refresh_zobj;
	refresh_zobj = (zend_object *)zend_object_store_get_object(refresh TSRMLS_CC);
	od_wrapper_object* refresh_od_obj = (od_wrapper_object*)refresh_zobj;
	od_igbinary_unserialize_data* refresh_igsd = &(refresh_od_obj->igsd);

	zend_object *replace_zobj;
	replace_zobj = (zend_object *)zend_object_store_get_object(replace TSRMLS_CC);
	od_wrapper_object* replace_od_obj = (od_wrapper_object*)replace_zobj;
	od_igbinary_unserialize_data* replace_igsd = &(replace_od_obj->igsd);

	// Check the version.
	if (memcmp(refresh_igsd->original_buffer, replace_igsd->original_buffer, OD_IGBINARY_VERSION_BYTES) != 0) {
		debug("od_refresh_odwrapper: version doesn't match, do nothing.");
		RETURN_BOOL(0);
	}

	refresh_igsd->buffer = replace_igsd->buffer;
	refresh_igsd->buffer_size = replace_igsd->buffer_size;
	refresh_igsd->buffer_offset = replace_igsd->buffer_offset;

	if (refresh_od_obj->od_properties) {
		od_hash_deinit(&refresh_od_obj->od_properties);
		refresh_od_obj->od_properties = NULL;
	}

	if (refresh_od_obj->zo.properties) {
		zend_hash_destroy(refresh_od_obj->zo.properties);
		FREE_HASHTABLE(refresh_od_obj->zo.properties);
		refresh_od_obj->zo.properties = NULL;
	}

	if (refresh_od_obj->zo.guards) {
		zend_hash_destroy(refresh_od_obj->zo.guards);
		FREE_HASHTABLE(refresh_od_obj->zo.guards);
		refresh_od_obj->zo.guards = NULL;
	}

	RETURN_BOOL(1);
}

PHP_FUNCTION(od_getobjectkeys_without_key)
{
	zval* obj = NULL;
	char *key = NULL;
	int key_len;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|s", &obj, &key, &key_len)) {
		RETURN_BOOL(0);
	}

	if(obj == NULL || Z_TYPE_P(obj) != IS_OBJECT || !IS_OD_WRAPPER(obj)) {
		RETURN_BOOL(0);
	}

	zend_object *zobj;
	zobj = (zend_object *)zend_object_store_get_object(obj TSRMLS_CC);
	od_wrapper_object* od_obj = (od_wrapper_object*)zobj;

	od_igbinary_unserialize_data* igsd = &(od_obj->igsd);

	HashTable *htable;
	ALLOC_HASHTABLE(htable);
	zend_hash_init(htable, 0, NULL, NULL, 0);

	int dummy = 1;

	char* name = NULL;
	uint32_t len = 0;
	long index;
	uint32_t first_key_offset = igsd->buffer_offset;
	while (igsd->buffer_offset < igsd->buffer_size)
	{
		od_igbinary_unserialize_get_key(igsd, &name, &len, &index);

		if(name==NULL || len==0) {
			od_error(E_ERROR, "key for object could not be null");
		}
		zend_hash_update(htable, name, len+1, &dummy, sizeof(int), NULL);

		od_wrapper_skip_value(igsd);
	}
	igsd->buffer_offset = first_key_offset;

	ODBucket* bkt = NULL;
	uint32_t i;
	if (od_obj->od_properties && od_obj->od_properties->buckets) {
		//add new properties here
		for (i=0; i<od_obj->od_properties->size; i++) {
			bkt = od_obj->od_properties->buckets + i;

			if (OD_IS_NEW(*bkt) && bkt->data!=NULL) {
				zend_hash_update(htable, (char*)bkt->key, bkt->key_len+1, &dummy, sizeof(int), NULL);
			} else if (OD_IS_UNSET(*bkt) && bkt) {
				zend_hash_del(htable, (char*)bkt->key, bkt->key_len+1);
			}
		}
	}

	Bucket* p = od_obj->zo.ce->default_properties.pListHead;
	while (p!=NULL) {
		if(p->pData && !zend_hash_quick_exists(htable, p->arKey, p->nKeyLength, p->h)) {
			if(!od_obj->od_properties ||
				(od_hash_find(od_obj->od_properties, p->arKey, p->nKeyLength-1, OD_HASH_VALUE(p->h), &bkt) == FAILURE 
					&& (!bkt || !OD_IS_OCCUPIED(*bkt)))) {
				zend_hash_quick_add(htable, p->arKey, p->nKeyLength, p->h, &dummy, sizeof(int), NULL);
			}
		}
		p = p->pListNext;
	}

	ulong hash_key = 0;
	if (key)
	{
		hash_key = zend_get_hash_value(key, key_len+1);
	}
	ulong p_hash;
	i = 0;
	int type;
	HashPosition pos;
	char *prop_name, *class_name;
	array_init(return_value);
    for(zend_hash_internal_pointer_reset_ex(htable, &pos);
		(type = zend_hash_get_current_key_ex(htable, &name, &len, &index, 0, &pos)) != HASH_KEY_NON_EXISTANT;
		zend_hash_move_forward_ex(htable, &pos))
    {
    	if (key && pos->h == hash_key && len == key_len+1)
		{
			if (memcmp(name, key, len) == 0) {
				continue;
			}
		}
		if (zend_check_property_access(zobj, name, len-1 TSRMLS_CC) == SUCCESS) {
			zend_unmangle_property_name(name, len-1, &class_name, &prop_name);
        	add_index_stringl(return_value, i, prop_name, strlen(prop_name), 1);
        	i += 1;
        }
    }

	zend_hash_destroy(htable);
	FREE_HASHTABLE(htable);
}

PHP_FUNCTION(od_get_mangled_name)
{
	zval* obj = NULL;
	char *name = NULL;
	int name_len;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|s", &obj, &name, &name_len)) {
		RETURN_NULL();
	}

	if(obj == NULL || Z_TYPE_P(obj) != IS_OBJECT) {
		RETURN_NULL();
	}

	if(IS_OD_WRAPPER(obj)) {
		od_error(E_ERROR, "od_get_mangled_name: work for non-odwrapper only for now");
		RETURN_NULL();
	}

	Z_TYPE_P(return_value) = IS_STRING;
	Z_STRVAL_P(return_value) = NULL;
	Z_STRLEN_P(return_value) = 0;

	zend_class_entry *ce = Z_OBJCE_P(obj);

	zval *v = NULL;
	HashTable *h = Z_OBJPROP_P(obj);
	char *low_level_name = name;
	int low_level_name_len = name_len;

	do {
		ulong hash = zend_get_hash_value(name, name_len + 1);
		if (zend_hash_quick_find(h, name, name_len + 1, hash, (void *) &v) == SUCCESS) {
			low_level_name = name;
			low_level_name_len = name_len;
		} else if (ce) {
			/* try private */
			char *priv_name = NULL;
			int priv_name_length = 0;

			zend_mangle_property_name(&priv_name, &priv_name_length, ce->name, ce->name_length,
						name, name_len, ce->type & ZEND_INTERNAL_CLASS);
			hash = zend_get_hash_value(priv_name, priv_name_length + 1);
			if (zend_hash_quick_find(h, priv_name, priv_name_length + 1, hash, (void *) &v) == SUCCESS) {
				low_level_name = priv_name;
				low_level_name_len = priv_name_length;
			} else {
				char *prot_name = NULL;
				int prot_name_length = 0;
				/* try protected */
				zend_mangle_property_name(&prot_name, &prot_name_length, "*", 1,
							name, name_len, ce->type & ZEND_INTERNAL_CLASS);
				hash = zend_get_hash_value(prot_name, prot_name_length + 1);
				if (zend_hash_quick_find(h, prot_name, prot_name_length + 1, hash, (void *) &v) == SUCCESS) {
					low_level_name = prot_name;
					low_level_name_len = prot_name_length;
				}
			}
		}

		// At most two '\0' encoded inside;
		char *mangled_name = emalloc(sizeof(char) * (low_level_name_len + 3));

		if (!mangled_name) {
			od_error(E_ERROR, "od_get_mangled_name: failed to allocate memory!");
			break;
		}

		char *p = low_level_name;
		char *q = mangled_name;
		while(low_level_name_len > 0) {
			if (*p == '\0') {
				*q++ = '\\';
				*q++ = '0';
			} else {
				*q++ = *p;
			}
			*p++;
			low_level_name_len--;
		}
		*q = '\0';

		Z_STRVAL_P(return_value) = mangled_name;
		Z_STRLEN_P(return_value) = strlen(mangled_name);
	} while (0);

	if (low_level_name != name) {
		efree(low_level_name);
	}
	return;
}

PHP_FUNCTION(od_release_memory)
{
	release_memory();
}

PHP_FUNCTION(od_reserialize)
{
	zval* z = NULL;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",&z)) {
		RETURN_NULL();
	}

	if(!IS_OD_WRAPPER(z)) {
		od_error(E_ERROR, "od_reserialize: argument must be ODWrapper!");
		RETURN_NULL();
	}

	Z_TYPE_P(return_value) = IS_STRING;
	Z_STRVAL_P(return_value) = NULL;
	Z_STRLEN_P(return_value) = 0;

	uint8_t *str = NULL;
	uint32_t str_len = 0;

	if (od_igbinary_serialize(&str, &str_len, z TSRMLS_CC)) {
		od_error(E_ERROR, "od_reserialize: serialize failed");
		RETURN_NULL();
	}

	if (str) {
		Z_STRVAL_P(return_value) = (char*)str;
		Z_STRLEN_P(return_value) = str_len;
		return;
	} else {
		RETURN_NULL();
	}
}
