
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_odus.h"
#include "odwrapper.h"

void test(){

	zend_class_entry* real_ce = zend_fetch_class(class_name,class_name_len,ZEND_FETCH_CLASS_DEFAULT);
	od_wrapper_object* od_obj = (od_wrapper_object*)zend_object_store_get_object(object TSRMLS_CC);
	od_obj->zo.ce = real_ce;

	static zend_object_handlers od_wrapper_object_handlers;

	od_wrapper_object_handlers.read_property = od_wrapper_read_property;
	od_wrapper_object_handlers.write_property = od_wrapper_write_property;
	od_wrapper_object_handlers.get_property_ptr_ptr = od_wrapper_get_property_ptr_ptr;




}

