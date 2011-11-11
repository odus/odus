/*
 *
 */

#ifndef PHP_ODWRAPPER_H
#define PHP_ODWRAPPER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "zend.h"
#include "php_odus.h"

#include "od_igbinary.h"

#include "od_debug.h"

// MACRO Definition

// PHP Related Definition

/* {{{ arginfo static macro */
#if PHP_MAJOR_VERSION > 5 || PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
#define ODUS_ARGINFO_STATIC
#else
#define ODUS_ARGINFO_STATIC static
#endif
/* }}} */

#define min(a,b) ((a)<(b)?(a):(b))

#define OD_WRAPPER_METHOD(func) \
	PHP_METHOD(ODWrapper, func)

// Module Definition

#define OD_MAGIC_NUM 12580

#define PHP_ODUS_OBJECT_NAME "ODWrapper"

#define OD_CREATE_FATAL do { \
	int*p = NULL; \
	*p=123; \
	} while(0)

#define od_error(type,fmt,args ...) OD_CREATE_FATAL; zend_error(type, "[ODWRAPPER (%d,%s,%s)] [ " fmt " ]\n", OD_LINE, OD_FUNCTION, OD_FILE, ##args)

#define OD_CALLL_INFO debug("### in %s for member '%s' of class '%s' (%s:%d:%s)",__FUNCTION__,Z_STRVAL_P(member),OD_CLASS_NAME(od_obj),OD_FILE,OD_LINE,OD_FUNCTION)


#define OD_FAIL od_error(E_ERROR,"bad data for unserialize")
// Utilities Definition

#define IS_OD_WRAPPER(object) (((object) == NULL || (object)->type != IS_OBJECT) ? 0 : (object)->value.obj.handlers == &od_wrapper_object_handlers)

#define OD_LINE (EG(opline_ptr)?((*EG(opline_ptr))?(*EG(opline_ptr))->lineno:-1):-1)
#define OD_FILE (EG(active_op_array)?EG(active_op_array)->filename:"")
#define OD_FUNCTION (EG(active_op_array)?EG(active_op_array)->function_name:"")

#define OD_CLASS_NAME(obj) obj->zo.ce->name

#define OD_MAX_REFCOUNT ((zend_uint)(-1))>>1

#define OD_IS_NEW_PROPERTY(p) (p == &EG(std_property_info))

#define SET_OD_REFCOUNT(val) ((val)->refcount = OD_MAX_REFCOUNT)


#define OD_OFFSET_POS(obj) ((obj->igsd).buffer + (obj->igsd).buffer_offset)
#define OD_NOT_END(obj) ((obj->igsd).buffer_offset < (obj->igsd).buffer_size)

#define OD_LOCAL_OFFSET_POS(igsd) ((igsd).buffer + (igsd).buffer_offset)
#define OD_LOCAL_OFFSET_LEN(igsd,end) (end - OD_LOCAL_OFFSET_POS(igsd))

#define OD_FAST_ALLOC(p,type) (p) = (type *) emalloc(sizeof(type))

#define OD_MAKE_KEY(k) OD_FAST_ALLOC(k,od_key)
#define OD_MAKE_POS(p) OD_FAST_ALLOC(p,od_pos)

#define OD_ZVAL_PTR_DTOR (void (*)(void *)) od_zval_ptr_dtor

#define OD_HASH_INIT(p) \
		ALLOC_HASHTABLE(p); \
		zend_hash_init(p, 0, NULL, OD_ZVAL_PTR_DTOR, 0);

#define OD_HASH_LAZY_INIT(p) do { \
		if(p==NULL) { \
			debug("lazy init " #p); \
			OD_HASH_INIT(p); \
			debug(#p " pointer %p",p); \
			} \
		} while(0)

#define OD_ARRAY_INIT(val) \
		val->type = IS_ARRAY; \
		OD_HASH_INIT(val->value.ht); \

#define OD_HASH_NUM(p) ((p==NULL)?0:(p)->nNumOfElements)

// Structure Definition

typedef struct {
	//FIXME
	//pointer will use 8 bytes in 64bit machine
	//will use int replace because our data is not so huge
	uint8_t* key_start;
	uint8_t* value_end;
} od_pos;

typedef struct {
	// XXX: the zend_object member must be the first one member !!!
	zend_object zo;

	//extra wrapper members
	// the serialized or semi-serialized data
	od_igbinary_unserialize_data igsd;

	HashTable* pos_info;
	HashTable *modified_properties;

	HashTable* obj_properties;

	zend_uchar initialized;
	zend_uchar all_get;

} od_wrapper_object;

// Function Definition
void od_wrapper_init(TSRMLS_D);
void od_wrapper_shutdown(TSRMLS_D);

#endif // PHP_ODWRAPPER_H
