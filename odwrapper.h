/*
 *
 */

#ifndef PHP_ODWRAPPER_H
#define PHP_ODWRAPPER_H

#include "od_def.h"
#include "od_hash.h"
#include "od_igbinary.h"
#include "php_odus.h"

#include "od_debug.h"

#define OD_WRAPPER_METHOD(func) \
	PHP_METHOD(ODWrapper, func)

#define PHP_ODUS_OBJECT_NAME "ODWrapper"

#define OD_FAIL od_error(E_ERROR,"bad data for unserialize")

#define OD_UPDATE_PROPERTY(persist,val,ret_bkt)	od_hash_update(od_obj->od_properties,property_info->name, property_info->name_length, OD_HASH_VALUE(property_info->h),persist,val,ret_bkt)

#define OD_FIND_PROPERTY od_hash_find(od_obj->od_properties, property_info->name, property_info->name_length, OD_HASH_VALUE(property_info->h), &bkt)

#define OD_SEARCH_PROPERTY(retval) \
				search_property(od_obj,property_info,&bkt); \
				OD_SET_RETVAL(retval); \

#define OD_SET_RETVAL(retval) (retval) = bkt? (zval**)(&bkt->data):NULL;


typedef struct {
	// XXX: the zend_object member must be the first one member !!!
	zend_object zo;

	//extra wrapper members
	// the serialized or semi-serialized data
	od_igbinary_unserialize_data igsd;

	ODHashTable* od_properties;

} od_wrapper_object;

// Function Definition
void od_wrapper_init(TSRMLS_D);
void od_wrapper_shutdown(TSRMLS_D);
void release_memory(TSRMLS_D);

inline uint8_t is_default(char* key, uint key_len, ulong hash, zval* val, HashTable* ht);

#endif // PHP_ODWRAPPER_H
