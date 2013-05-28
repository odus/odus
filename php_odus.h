
#ifndef PHP_ODUS_H
#define PHP_ODUS_H

#include "od_def.h"

extern zend_class_entry *odus_exception_ce;

extern zend_module_entry odus_module_entry;
#define phpext_odus_ptr &odus_module_entry

#ifdef PHP_WIN32
#define PHP_ODUS_API __declspec(dllexport)
#else
#define PHP_ODUS_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#define ALLOW_CLASS_SERIALIZE 0

PHP_MINIT_FUNCTION(odus);
PHP_MSHUTDOWN_FUNCTION(odus);
PHP_RINIT_FUNCTION(odus);
PHP_RSHUTDOWN_FUNCTION(odus);
PHP_MINFO_FUNCTION(odus);

PHP_FUNCTION(od_serialize);
PHP_FUNCTION(od_unserialize);
PHP_FUNCTION(od_version);
PHP_FUNCTION(od_format_version);
PHP_FUNCTION(od_format_match);
PHP_FUNCTION(od_overwrite_function);
PHP_FUNCTION(od_refresh_odwrapper);
PHP_FUNCTION(od_getobjectkeys_without_key);
PHP_FUNCTION(od_release_memory);
PHP_FUNCTION(od_reserialize);

/** Binary protocol version of igbinary. */
#define OD_IGBINARY_FORMAT_FLAG				0x4F440000		// ASCII "OD"
#define OD_IGBINARY_FORMAT_VERSION_MASK		0x0000FFFF
#define OD_IGBINARY_FORMAT_VERSION_01		0x1
#define OD_IGBINARY_FORMAT_VERSION_02		0x2


#define OD_IGBINARY_FORMAT_HEADER 			(OD_IGBINARY_FORMAT_FLAG | OD_IGBINARY_FORMAT_VERSION_02)     // current version

#define OD_IGBINARY_STATIC_STRING_COMMENT_CHAR     '#'

#define OD_IGBINARY_DEFAULT_STATIC_STRINGS_FILE    "/var/www/releases/current/game/odus/static_strings.ini"

#define _TEXT(t) #t
#define TEXT(t) _TEXT(t)

#define OD_IGBINARY_VERSION_BYTES  4

#define OD_VERSION "2.0.2"

ZEND_BEGIN_MODULE_GLOBALS(odus)
	zend_bool remove_default;
	zend_bool od_throw_exceptions;
	zend_bool od_reduce_fatals;
	zend_bool format_version;
	zend_bool force_release_memory;
   char * static_strings_file;      /* path of static strings file. */
ZEND_END_MODULE_GLOBALS(odus)

/* In every utility function you add that needs to use variables 
   in php_odus_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as ODUS_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define ODUS_G(v) TSRMG(odus_globals_id, zend_odus_globals *, v)
#else
#define ODUS_G(v) (odus_globals.v)
#endif

#ifdef PHP_WIN32
#define PHP_ODUS_API __declspec(dllexport)
#else
#define PHP_ODUS_API
#endif

#endif	/* PHP_ODUS_H */

