
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_odus.h"

#include "odwrapper.h"
#include "ext/standard/php_incomplete_class.h"

//FIXME
//to be removed
#include "ext/standard/php_var.h"
#include "ext/standard/php_smart_str.h"

/* If you declare any globals in php_odus.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(odus)
*/

/* True global resources - no need for thread safety here */
static int le_odus;

/* {{{ odus_functions[]
 *
 * Every user visible function must have an entry in odus_functions[].
 */
zend_function_entry odus_functions[] = {
	PHP_FE(od_serialize,	NULL)
	PHP_FE(od_unserialize,	NULL)
	PHP_FE(od_overwrite_function,	NULL)
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
	PHP_RINIT(odus),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(odus),	/* Replace with NULL if there's nothing to do at request end */
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

void normal_od_wrapper_serialize(od_igbinary_serialize_data* igsd, zval* obj, zend_uchar is_root);

extern zend_object_handlers od_wrapper_object_handlers;

extern void jump_member_key(char** pos_p);
extern int check_od_wrapper_ob_status(zval* obj);
extern void od_wrapper_hash_apply_with_argument(HashTable *ht, apply_func_arg_t apply_func, void *argument TSRMLS_DC);

//FIXME
// to be removed
 inline ulong my_inline_hash_func(char *arKey, uint nKeyLength)
{
	register ulong hash = 5381;

	/* variant with the hash unrolled eight times */
	for (; nKeyLength >= 8; nKeyLength -= 8) {
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
	}
	switch (nKeyLength) {
		case 7: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
		case 6: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
		case 5: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
		case 4: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
		case 3: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
		case 2: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
		case 1: hash = ((hash << 5) + hash) + *arKey++; break;
		case 0: break;
	}
	return hash;
}

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

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("odus.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_odus_globals, odus_globals)
    STD_PHP_INI_ENTRY("odus.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_odus_globals, odus_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_odus_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_odus_init_globals(zend_odus_globals *odus_globals)
{
	odus_globals->global_value = 0;
	odus_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(odus)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/

	od_wrapper_init(TSRMLS_C);

	//FIXME
	//doesn't do this now
	//od_overwrite_function("serialize","od_serialize");

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(odus)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/

	od_wrapper_shutdown(TSRMLS_C);

	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(odus)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(odus)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(odus)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "odus support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

int od_wrapper_remove_for_sleep(void* bkt, void* arg)
{
	assert(arg);

	if(!bkt || !arg) {
		return ZEND_HASH_APPLY_REMOVE;
	}

	Bucket* p = (Bucket*)bkt;

	HashTable** p_ht = (HashTable**)arg;

	//keep unset property
	return (p->pDataPtr==NULL || zend_hash_quick_exists(*p_ht, p->arKey, p->nKeyLength, p->h))?ZEND_HASH_APPLY_KEEP:ZEND_HASH_APPLY_REMOVE;
}

void normal_php_serialize(smart_str* buf, zval* obj)
{
	if(buf==NULL || obj==NULL) return;

	HashTable var_hash;

	PHP_VAR_SERIALIZE_INIT(var_hash);
	php_var_serialize(buf, &obj, &var_hash TSRMLS_CC);
	PHP_VAR_SERIALIZE_DESTROY(var_hash);
}

inline void deal_with_unmodified_object(od_igbinary_serialize_data* igsd, od_wrapper_object* od_obj, zend_uchar is_root)
{
	if(is_root) {
		igsd->scalar = 1;
		igsd->objects.data = NULL;

		igsd->buffer_size = od_obj->igsd.buffer_size + OD_IGBINARY_VERSION_BYTES;
		igsd->buffer_capacity = igsd->buffer_size + 1;

		igsd->buffer = (uint8_t*)emalloc(igsd->buffer_capacity);

		memcpy(igsd->buffer, od_obj->igsd.buffer - OD_IGBINARY_VERSION_BYTES, igsd->buffer_size);
		igsd->buffer[igsd->buffer_size] = 0;
	} else {
		od_igbinary_serialize_memcpy(igsd,od_obj->igsd.buffer,od_obj->igsd.buffer_size);
	}
}

HashTable* flip_sleep_array_hash(HashTable* h, zend_class_entry* ce) {

	if(!h) return NULL;

	HashTable* flip_h;
	ALLOC_HASHTABLE(flip_h);

	zend_hash_init(flip_h, h->nNumOfElements, NULL, NULL, 0);

	Bucket* p = h->pListHead;

	zval* data = NULL;

	void* value = NULL;

	//zend_hash_quick_find(&ce->properties_info, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, h, (void **) &property_info)==SUCCESS

	ulong hash;

	zend_property_info *property_info = NULL;

	while(p!=NULL) {

		data = (zval*) p->pDataPtr;

		if(data) {
			if(data->type != IS_STRING) {
				od_error(E_ERROR,"item of sleep array must be string type");
			} else {

				hash = zend_get_hash_value(Z_STRVAL_P(data),Z_STRLEN_P(data)+1);

				if(zend_hash_quick_find(&ce->properties_info,Z_STRVAL_P(data),Z_STRLEN_P(data)+1, hash, (void **) &property_info)==SUCCESS) {
					zend_hash_quick_add(flip_h,property_info->name,property_info->name_length+1,property_info->h,&value,sizeof(void*),NULL);
				} else {
					zend_hash_quick_add(flip_h,Z_STRVAL_P(data),Z_STRLEN_P(data)+1,hash,&value,sizeof(void*),NULL);
				}
			}
		}

		p = p->pListNext;
	}

	return flip_h;
}

void normal_od_wrapper_serialize(od_igbinary_serialize_data* igsd, zval* obj, zend_uchar is_root)
{
	if(obj==NULL) {
		od_error(E_ERROR, "obj could not be NULL here");
		return;
	}

	od_wrapper_object* od_obj = NULL;

	if(IS_OD_WRAPPER(obj)) {
		od_obj=(od_wrapper_object*)zend_object_store_get_object(obj);
	}

	if(!od_obj) {
		if(obj->type == IS_ARRAY) {
			od_igbinary_serialize_array(igsd,obj,0,0,1);
		} else {
			// no od_obj means that is_root is false, so igsd has been initialized
			od_igbinary_serialize_zval(igsd,obj);
		}
	}else{

		if(is_root) {
			check_od_wrapper_ob_status(obj);
		}

		if(OD_HASH_NUM(od_obj->modified_properties)==0){

			deal_with_unmodified_object(igsd, od_obj, is_root);

			return;
		}else{

			if(od_obj->zo.ce && od_obj->zo.ce != PHP_IC_ENTRY && zend_hash_exists(&od_obj->zo.ce->function_table, "__sleep", sizeof("__sleep"))) {

				/* function name string */
				zval f;
				zval* h = NULL;
				HashTable* flip_h = NULL;

				INIT_PZVAL(&f);
				ZVAL_STRINGL(&f, "__sleep", sizeof("__sleep") - 1, 0);

				if (call_user_function_ex(CG(function_table), &obj, &f, &h, 0, 0, 1, NULL TSRMLS_CC) == SUCCESS && !EG(exception)) {

					if (h) {
						if (Z_TYPE_P(h) == IS_ARRAY) {

							//FIXME
							debug("modified properties before sleep");
							print_ht(od_obj->modified_properties);

							flip_h = flip_sleep_array_hash(Z_ARRVAL_P(h),od_obj->zo.ce);

							debug("sleep array for class %s",OD_CLASS_NAME(od_obj));
							print_ht(flip_h);

							od_wrapper_hash_apply_with_argument(od_obj->modified_properties,od_wrapper_remove_for_sleep,&flip_h);

							debug("modified properties after sleep");
							print_ht(od_obj->modified_properties);

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

				if(flip_h) {
					zend_hash_destroy(flip_h);
					FREE_HASHTABLE(flip_h);
				}
			}

			//debug("pos info for class %s",OD_CLASS_NAME(od_obj));
			//print_ht(od_obj->pos_info);

			debug("obj info for class %s",OD_CLASS_NAME(od_obj));
			print_ht(od_obj->obj_properties);

			debug("modified info for class %s (modified num: %d)",OD_CLASS_NAME(od_obj),OD_HASH_NUM(od_obj->modified_properties));

			print_ht(od_obj->modified_properties);

			//debug("property info for class %s",OD_CLASS_NAME(od_obj));
			//print_ht(od_obj->zo.properties);


			if(is_root) {
				//We need do necessary initialization
				assert(igsd->buffer==NULL);
				assert(igsd->buffer_size==0);
				assert(igsd->buffer_capacity==0);

				igsd->buffer = NULL;
				igsd->buffer_size = 0;
				igsd->buffer_capacity = od_obj->igsd.buffer_size + min(OD_RESERVED_BUFFER_LEN,od_obj->igsd.buffer_size>>1);

				igsd->buffer = (uint8_t *) emalloc(igsd->buffer_capacity);
				if (igsd->buffer == NULL) {
					od_error(E_ERROR,"failed memory allocation");
				}

				igsd->scalar = 0;
				hash_si_init(&igsd->objects, 1);

				//add header here
				od_igbinary_serialize_header(igsd);

			}

			debug("deal with changed part of object %s",OD_CLASS_NAME(od_obj));

			od_igbinary_unserialize_data local_igsd = od_obj->igsd;
			local_igsd.buffer_offset = 0;

			char* class_name;
			size_t class_name_len;

			od_igbinary_unserialize_chararray(&local_igsd, od_igbinary_get_type(&local_igsd), &class_name, &class_name_len);

			//XXX
			//this position will not changed
			size_t local_array_info_offset = local_igsd.buffer_offset;

			int member_num = od_igbinary_get_member_num(&local_igsd,od_igbinary_get_type(&local_igsd));

			//XXX
			//this position may be changed latter
			size_t local_len_info_offset = local_igsd.buffer_offset;

			od_igbinary_skip_value_len(&local_igsd);

			// get number of unset properties
			int diff_num = 0;

			debug("number of modified properties: %d",OD_HASH_NUM(od_obj->modified_properties));

			Bucket* p = NULL;

			p = od_obj->modified_properties->pListHead;

			while(p!=NULL) {

				if(!od_obj->pos_info || !zend_hash_quick_exists(od_obj->pos_info, p->arKey, p->nKeyLength, p->h)) {
					// new property
					if(p->pDataPtr != NULL) {
						diff_num ++;
					}
				} else {
					if(p->pDataPtr == NULL) {
						diff_num --;
					}
				}

				p = p->pListNext;
			}

			if(diff_num !=0) {

				debug("diff num: %d",diff_num);

				int new_member_num = member_num + diff_num;

				if(new_member_num == member_num) {
					od_igbinary_serialize_memcpy(igsd,local_igsd.buffer, local_len_info_offset);
				} else {

					od_igbinary_serialize_memcpy(igsd,local_igsd.buffer, local_array_info_offset);

					debug("new meber num is %d",new_member_num);

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
				}
			} else {
				od_igbinary_serialize_memcpy(igsd,local_igsd.buffer, local_len_info_offset);
			}

			size_t g_len_info_pos = igsd->buffer_size;

			od_igbinary_serialize_skip_n(igsd,OD_IGBINARY_VALUE_LEN_SIZE);


			int modified_num = OD_HASH_NUM(od_obj->modified_properties);
			int check_num = 0;

			debug("deal with modified properties");

			p = od_obj->pos_info?od_obj->pos_info->pListHead:NULL;

			while(p!=NULL) {

				zval*** data_p = NULL;

				if(zend_hash_quick_find(od_obj->modified_properties,p->arKey,p->nKeyLength,p->h,(void**)&data_p)==SUCCESS) {

					od_pos* pos = *(od_pos**)p->pData;

					if(data_p != NULL && *data_p != NULL) {

						debug("find pos info for modified key '%s' data_p: %p for class %s",p->arKey, **data_p, OD_CLASS_NAME(od_obj));

						od_igbinary_serialize_memcpy(igsd, OD_LOCAL_OFFSET_POS(local_igsd), OD_LOCAL_OFFSET_LEN(local_igsd, pos->key_start));

						local_igsd.buffer_offset = pos->key_start - local_igsd.buffer;
						size_t tmp_pos = local_igsd.buffer_offset;
						od_igbinary_unserialize_skip_key(&local_igsd);

						//copy key
						od_igbinary_serialize_memcpy(igsd, pos->key_start, local_igsd.buffer_offset - tmp_pos);

						//serialize value
						normal_od_wrapper_serialize(igsd,**data_p,0);

						local_igsd.buffer_offset = pos->value_end - local_igsd.buffer;
					}
					else {
						debug("key '%s' is unset for class %s",p->arKey, OD_CLASS_NAME(od_obj));

						od_igbinary_serialize_memcpy(igsd, OD_LOCAL_OFFSET_POS(local_igsd), OD_LOCAL_OFFSET_LEN(local_igsd, pos->key_start));

						local_igsd.buffer_offset = pos->value_end - local_igsd.buffer;
					}

					check_num ++;

					if(check_num >= modified_num) {
						debug("break because check_num(%d) >= modified_num(%d) for class %s",check_num,modified_num,OD_CLASS_NAME(od_obj));
						break;
					}
				}

				p = p->pListNext;
			}

			od_igbinary_serialize_memcpy(igsd, OD_LOCAL_OFFSET_POS(local_igsd), local_igsd.buffer_size - local_igsd.buffer_offset);

			// deal with new properties

			debug("deal with new properties for class %s",OD_CLASS_NAME(od_obj));

			p = od_obj->modified_properties->pListHead;

			while(p!=NULL) {

				if(!od_obj->pos_info || !zend_hash_quick_exists(od_obj->pos_info, p->arKey, p->nKeyLength, p->h)) {
					if(p->pDataPtr != NULL) {

						if(p->nKeyLength == 0) {
							od_igbinary_serialize_long(igsd, p->h);
						} else {
							od_igbinary_serialize_string(igsd, p->arKey, p->nKeyLength-1);
						}

						normal_od_wrapper_serialize(igsd, **((zval***)p->pData),0);

					}
				}

				p = p->pListNext;
			}

			//modify value len
			od_igbinary_serialize_value_len(igsd, igsd->buffer_size - g_len_info_pos - OD_IGBINARY_VALUE_LEN_SIZE,g_len_info_pos);
		}
	}
}

/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string od_serialize(object ODWrapper, persistent=0)
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

	//debug("in od_serialize => file: %s function: %s line: %d",OD_FILE,OD_FUNCTION,OD_LINE);

	od_igbinary_serialize_data igsd = {0};

	if(IS_OD_WRAPPER(z)) {

		normal_od_wrapper_serialize(&igsd,z,1);

	} else {
		if (od_igbinary_serialize_data_init(&igsd, Z_TYPE_P(z) != IS_OBJECT && Z_TYPE_P(z) != IS_ARRAY TSRMLS_CC)) {
			od_error(E_ERROR, "od_igbinary_serialize: cannot init igsd");
			RETURN_NULL();
		}

		if (od_igbinary_serialize_header(&igsd TSRMLS_CC) != 0) {
			od_error(E_ERROR, "od_igbinary_serialize: cannot write header");
			od_igbinary_serialize_data_deinit(&igsd TSRMLS_CC);
			RETURN_NULL();
		}

		if (od_igbinary_serialize_zval(&igsd, z TSRMLS_CC) != 0) {
			od_igbinary_serialize_data_deinit(&igsd TSRMLS_CC);
			RETURN_NULL();
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
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
