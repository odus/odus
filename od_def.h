/*
 *
 */

#ifndef OD_DEF_H
#define OD_DEF_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "zend.h"
#include "zend_alloc.h"
#include "zend_globals.h"
#include "zend_interfaces.h"
#include "zend_exceptions.h"
#include "php_streams.h"
#include "zend_object_handlers.h"
#include "ext/standard/php_var.h"
#include "ext/standard/php_smart_str.h"
#include "ext/standard/php_incomplete_class.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>
#include <sys/types.h>

#define TKEY "lastEnergyCheck"
//#define TKEY "commodities"

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

#ifdef OD_DEBUG
	#define od_error(type,fmt,args ...) OD_CREATE_FATAL; zend_error(type, "[ODWRAPPER (%d,%s,%s)] [ " fmt " ]\n", OD_LINE, OD_FUNCTION, OD_FILE, ##args)
#else
	#define od_error(type,fmt,args ...) zend_error(type, "[ODWRAPPER (%d,%s,%s)] [ " fmt " ]\n", OD_LINE, OD_FUNCTION, OD_FILE, ##args)
#endif

#define OD_CALL_INFO debug("### in %s for member '%s' of class '%s' (%s:%d:%s)",__FUNCTION__,Z_STRVAL_P(member),OD_CLASS_NAME(od_obj),OD_FILE,OD_LINE,OD_FUNCTION)

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

#define OD_ZVAL_PTR_DTOR od_zval_ptr_dtor
#define OD_ZVAL_PTR_DTOR_FUNC (void (*)(void *)) OD_ZVAL_PTR_DTOR

#define OD_HASH_INIT(p) \
		ALLOC_HASHTABLE(p); \
		zend_hash_init(p, 0, NULL, OD_ZVAL_PTR_DTOR_FUNC, 0);

#define OD_HASH_LAZY_INIT(p) do { \
		if(p==NULL) { \
			nodebug("lazy init " #p); \
			OD_HASH_INIT(p); \
			nodebug(#p " pointer %p",p); \
			} \
		} while(0)

#define OD_ARRAY_INIT(val) \
		val->type = IS_ARRAY; \
		OD_HASH_INIT(val->value.ht); \

#define OD_HASH_NUM(p) ((p==NULL)?0:(p)->nNumOfElements)

// Structure Definition

typedef struct {
	zval* data;
	uint32_t key_start_offset;
	uint32_t val_start_offset;
	uint32_t val_end_offset;
} member_pos;

// Function Definition

void od_zval_ptr_dtor(zval **zval_ptr);

#endif // OD_DEF_H
