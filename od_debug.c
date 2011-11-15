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


#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include <sys/time.h>

//FIXME
//to be removed
#include "SAPI.h"

#include "od_debug.h"

#ifdef OD_DEBUG

struct timeval tp = {0};

double start=0;
double prev=0;

uint32_t start_peak=0;
uint32_t prev_peak=0;

double get_time() 
{
	if(gettimeofday(&tp, NULL)) {
		return 0;
	}
	
	return (double)(tp.tv_sec + tp.tv_usec / 1000000.00);
}


void debug_buffer(uint8_t* buffer, uint32_t len, uint32_t pos)
{
	//FIXME
	//return;

	debug("in debug_buffer");

	char c = ' ';

	int colum_len = 24;

	int padding_num = (len%colum_len)? (colum_len - len%colum_len):0;

	FILE* fp = fopen("/tmp/buffer","a+");

	if(!fp) return;

	fwrite(buffer,1,len,fp);

	uint32_t i;

	for(i=0;i<padding_num;i++) {
		fwrite(&c,1,1,fp);
	}

	if(pos>0) {

		char c1='-';
		char c2='>';

		for(i=0;i+1<pos;i++) {
			fwrite(&c1,1,1,fp);
		}

		if(pos>0) {
			fwrite(&c2,1,1,fp);
		}

		fwrite(buffer+pos,1,len-pos,fp);


		for(i=0;i<padding_num;i++) {
			fwrite(&c,1,1,fp);
		}
	}

	fclose(fp);
}

void debug_mem(char* format, ...)
{

	va_list args;

	va_start(args, format);

	fprintf(stderr, "\t\t[ ");
	vfprintf(stderr, format, args);
	fprintf(stderr, " ]\n");

	va_end(args);

	uint32_t current_peak = zend_memory_peak_usage(1);

	if(start_peak<1) {
		start_peak = current_peak;
	} else {
#if OD_DEBUG_MEM
		if(current_peak != prev_peak) {
			fprintf(stderr, "\t\t[ till now memory: %u (%.3fM)",current_peak - start_peak,(current_peak - start_peak)/(1024.0*1024.0));
			fprintf(stderr, " this time memory: %u  (%.3fM)]\n",current_peak - prev_peak,(current_peak - prev_peak)/(1024.0*1024.0));
		}
#endif
	}

	prev_peak = current_peak;
}

void debug(char* format, ...)
{
	double current = get_time();

	if(start<10e-4) {
		start = current;
	} else {
#if OD_DEBUG_TIME
		fprintf(stderr, "\t\t[ till now time: %.5f ",current - start);
		fprintf(stderr, " this time: %.5f ]\n",current - prev);
#endif
	}

	prev = current;


	uint32_t current_peak = zend_memory_peak_usage(1);

	if(start_peak<1) {
		start_peak = current_peak;
	} else {
#if OD_DEBUG_MEM
		if(current_peak != prev_peak) {
			fprintf(stderr, "\t\t[ till now memory: %u (%.3fM)",current_peak - start_peak,(current_peak - start_peak)/(1024.0*1024.0));
			fprintf(stderr, " this time memory: %u  (%.3fM)]\n",current_peak - prev_peak,(current_peak - prev_peak)/(1024.0*1024.0));
		}
#endif
	}

	prev_peak = current_peak;

	char* client="cli";

	int type = E_WARNING;

	if(sapi_module.name && strcmp(sapi_module.name,client) ==0) {

		va_list args;

		va_start(args, format);

		fprintf(stderr, "\t\t[ ");
		vfprintf(stderr, format, args);
		fprintf(stderr, " ]\n");

		va_end(args);
	} else {

		va_list args;
		va_list usr_copy;
		zval ***params;
		zval *retval;
		zval *z_error_type, *z_error_message, *z_error_filename, *z_error_lineno, *z_context;
		char *error_filename;
		uint error_lineno;
		zval *orig_user_error_handler;
		zend_bool in_compilation;
		zend_class_entry *saved_class_entry;
		TSRMLS_FETCH();

		/* Obtain relevant filename and lineno */
		switch (type) {
			case E_CORE_ERROR:
			case E_CORE_WARNING:
				error_filename = NULL;
				error_lineno = 0;
				break;
			case E_PARSE:
			case E_COMPILE_ERROR:
			case E_COMPILE_WARNING:
			case E_ERROR:
			case E_NOTICE:
			case E_STRICT:
			case E_WARNING:
			case E_USER_ERROR:
			case E_USER_WARNING:
			case E_USER_NOTICE:
			case E_RECOVERABLE_ERROR:
				if (zend_is_compiling(TSRMLS_C)) {
					error_filename = zend_get_compiled_filename(TSRMLS_C);
					error_lineno = zend_get_compiled_lineno(TSRMLS_C);
				} else if (zend_is_executing(TSRMLS_C)) {
					error_filename = zend_get_executed_filename(TSRMLS_C);
					error_lineno = zend_get_executed_lineno(TSRMLS_C);
				} else {
					error_filename = NULL;
					error_lineno = 0;
				}
				break;
			default:
				error_filename = NULL;
				error_lineno = 0;
				break;
		}
		if (!error_filename) {
			error_filename = "Unknown";
		}

		va_start(args, format);

		/* if we don't have a user defined error handler */
		if (!EG(user_error_handler)
			|| !(EG(user_error_handler_error_reporting) & type)) {
			zend_error_cb(type, error_filename, error_lineno, format, args);
		} else switch (type) {
			case E_ERROR:
			case E_PARSE:
			case E_CORE_ERROR:
			case E_CORE_WARNING:
			case E_COMPILE_ERROR:
			case E_COMPILE_WARNING:
				/* The error may not be safe to handle in user-space */
				zend_error_cb(type, error_filename, error_lineno, format, args);
				break;
			default:
				/* Handle the error in user space */
				ALLOC_INIT_ZVAL(z_error_message);
				ALLOC_INIT_ZVAL(z_error_type);
				ALLOC_INIT_ZVAL(z_error_filename);
				ALLOC_INIT_ZVAL(z_error_lineno);
				ALLOC_INIT_ZVAL(z_context);

	/* va_copy() is __va_copy() in old gcc versions.
	 * According to the autoconf manual, using
	 * memcpy(&dst, &src, sizeof(va_list))
	 * gives maximum portability. */
	#ifndef va_copy
	# ifdef __va_copy
	#  define va_copy(dest, src)	__va_copy((dest), (src))
	# else
	#  define va_copy(dest, src)	memcpy(&(dest), &(src), sizeof(va_list))
	# endif
	#endif
				va_copy(usr_copy, args);
				z_error_message->value.str.len = zend_vspprintf(&z_error_message->value.str.val, 0, format, usr_copy);
	#ifdef va_copy
				va_end(usr_copy);
	#endif
				z_error_message->type = IS_STRING;

				z_error_type->value.lval = type;
				z_error_type->type = IS_LONG;

				if (error_filename) {
					z_error_filename->value.str.len = strlen(error_filename);
					z_error_filename->value.str.val = estrndup(error_filename, z_error_filename->value.str.len);
					z_error_filename->type = IS_STRING;
				}

				z_error_lineno->value.lval = error_lineno;
				z_error_lineno->type = IS_LONG;

				z_context->value.ht = EG(active_symbol_table);
				z_context->type = IS_ARRAY;
				zval_copy_ctor(z_context);

				params = (zval ***) emalloc(sizeof(zval **)*5);
				params[0] = &z_error_type;
				params[1] = &z_error_message;
				params[2] = &z_error_filename;
				params[3] = &z_error_lineno;
				params[4] = &z_context;

				orig_user_error_handler = EG(user_error_handler);
				EG(user_error_handler) = NULL;

				/* User error handler may include() additinal PHP files.
				 * If an error was generated during comilation PHP will compile
				 * such scripts recursivly, but some CG() variables may be
				 * inconsistent. */

				in_compilation = zend_is_compiling(TSRMLS_C);
				if (in_compilation) {
					saved_class_entry = CG(active_class_entry);
					CG(active_class_entry) = NULL;
				}

				if (call_user_function_ex(CG(function_table), NULL, orig_user_error_handler, &retval, 5, params, 1, NULL TSRMLS_CC)==SUCCESS) {
					if (retval) {
						if (Z_TYPE_P(retval) == IS_BOOL && Z_LVAL_P(retval) == 0) {
							zend_error_cb(type, error_filename, error_lineno, format, args);
						}
						zval_ptr_dtor(&retval);
					}
				} else if (!EG(exception)) {
					/* The user error handler failed, use built-in error handler */
					zend_error_cb(type, error_filename, error_lineno, format, args);
				}

				if (in_compilation) {
					CG(active_class_entry) = saved_class_entry;
				}

				if (!EG(user_error_handler)) {
					EG(user_error_handler) = orig_user_error_handler;
				}
				else {
					zval_ptr_dtor(&orig_user_error_handler);
				}

				efree(params);
				zval_ptr_dtor(&z_error_message);
				zval_ptr_dtor(&z_error_type);
				zval_ptr_dtor(&z_error_filename);
				zval_ptr_dtor(&z_error_lineno);
				zval_ptr_dtor(&z_context);
				break;
		}

		va_end(args);

		if (type == E_PARSE) {
			EG(exit_status) = 255;
			zend_init_compiler_data_structures(TSRMLS_C);
		}
	}
}

void print_zval(zval* val) {

	smart_str buf = {0};
	php_serialize_data_t var_hash;

	PHP_VAR_SERIALIZE_INIT(var_hash);
	php_var_serialize(&buf, val, &var_hash TSRMLS_CC);
	PHP_VAR_SERIALIZE_DESTROY(var_hash);

	if (buf.c) {
		uint32_t i;
		for(i=0; i<buf.len; i++) {
			if(buf.c[i]=='\0') buf.c[i]='0';
		}

		debug(buf.c);
	} else {
		debug("null value");
	}
}

void print_ht(HashTable* ht)
{
	print_ht_ex(ht,0);
}

void print_ht_ex(HashTable* ht,zend_uchar print_val)
{
	if(ht==NULL) return;

	Bucket* p = ht->pListHead;

	while(p!=NULL) {

		if(p->nKeyLength == 0) {
			debug("hash: %u (index key)",p->h);

			if(p->pDataPtr == NULL) {
				debug('nill');
			} else {
				zval* val = (zval*)p->pDataPtr;

				debug("value string: %s",Z_STRVAL_P(val));
				print_zval((zval*)p->pDataPtr);
			}
		} else {

			char tmpc[256];

			int i;

			for(i=0;i<p->nKeyLength-1;i++) {
				tmpc[i]=p->arKey[i]?p->arKey[i]:'0';
			}

			tmpc[p->nKeyLength-1]='\0';

			if(print_val) {
				debug("hash: %u len: %d name: %s value:",p->h, p->nKeyLength-1, tmpc);

				if(p->pDataPtr == NULL) {
					debug('nill');
				} else {
					print_zval((zval*)p->pDataPtr);
				}
			} else {
				debug("hash: %u len: %d name: %s",p->h, p->nKeyLength-1, tmpc);
			}
		}

		p = p->pListNext;
	}
}

static int od_verify_property_access(zend_property_info *property_info, zend_class_entry *ce TSRMLS_DC)
{
	switch (property_info->flags & ZEND_ACC_PPP_MASK) {
		case ZEND_ACC_PUBLIC:
			return 1;
		case ZEND_ACC_PROTECTED:
			return zend_check_protected(property_info->ce, EG(scope));
		case ZEND_ACC_PRIVATE:
			if ((ce==EG(scope) || property_info->ce == EG(scope)) && EG(scope)) {
				return 1;
			} else {
				return 0;
			}
			break;
	}
	return 0;
}

static inline zend_bool od_is_derived_class(zend_class_entry *child_class, zend_class_entry *parent_class)
{
	child_class = child_class->parent;
	while (child_class) {
		if (child_class == parent_class) {
			return 1;
		}
		child_class = child_class->parent;
	}

	return 0;
}

struct _zend_property_info *od_get_property_info(zend_class_entry *ce, zval *member, int silent TSRMLS_DC)
{
	debug("in %s member: %s",__FUNCTION__,Z_STRVAL_P(member));

	zend_property_info *property_info = NULL;
	zend_property_info *scope_property_info;
	zend_bool denied_access = 0;
	ulong h;

	if (Z_STRVAL_P(member)[0] == '\0') {
		if (!silent) {
			if (Z_STRLEN_P(member) == 0) {
				zend_error(E_ERROR, "Cannot access empty property");
			} else {
				zend_error(E_ERROR, "Cannot access property started with '\\0'");
			}
		}
		return NULL;
	}
	h = zend_get_hash_value(Z_STRVAL_P(member), Z_STRLEN_P(member) + 1);

	debug("in %s hash: %u",__FUNCTION__,h);

	if (zend_hash_quick_find(&ce->properties_info, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, h, (void **) &property_info)==SUCCESS) {

		debug("in %s: member: %s member_len: %d hash: %u",__FUNCTION__,Z_STRVAL_P(member), Z_STRLEN_P(member), h);
		debug("in %s ps0: name: %s len: %d hash: %u",__FUNCTION__,property_info->name, property_info->name_length, property_info->h);

		if(property_info->flags & ZEND_ACC_SHADOW) {
			debug("in %s ps0",__FUNCTION__);
			/* if it's a shadow - go to access it's private */
			property_info = NULL;
		} else {

			print_ht(&ce->properties_info);

			if (od_verify_property_access(property_info, ce TSRMLS_CC)) {
				debug("in %s ps02",__FUNCTION__);
				if (property_info->flags & ZEND_ACC_CHANGED
					&& !(property_info->flags & ZEND_ACC_PRIVATE)) {
					/* We still need to make sure that we're not in a context
					 * where the right property is a different 'statically linked' private
					 * continue checking below...
					 */
					debug("in %s ps03",__FUNCTION__);
				} else {
					debug("in %s ps04",__FUNCTION__);
					if (!silent && (property_info->flags & ZEND_ACC_STATIC)) {
						zend_error(E_STRICT, "Accessing static property %s::$%s as non static", ce->name, Z_STRVAL_P(member));
					}
					debug("in %s ps05: name: %s len: %d hash: %u",__FUNCTION__,property_info->name, property_info->name_length, property_info->h);
					return property_info;
				}
			} else {
				/* Try to look in the scope instead */
				denied_access = 1;
			}
		}
	}
	if (EG(scope) != ce
		&& od_is_derived_class(ce, EG(scope))
		&& EG(scope)
		&& zend_hash_quick_find(&EG(scope)->properties_info, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, h, (void **) &scope_property_info)==SUCCESS
		&& scope_property_info->flags & ZEND_ACC_PRIVATE) {
		debug("in %s ps1",__FUNCTION__);
		return scope_property_info;
	} else if (property_info) {
		debug("in %s ps2",__FUNCTION__);
		if (denied_access) {
			debug("in %s ps21",__FUNCTION__);
			/* Information was available, but we were denied access.  Error out. */
			if (silent) {
				return NULL;
			}
			zend_error(E_ERROR, "Cannot access %s property %s::$%s", zend_visibility_string(property_info->flags), ce->name, Z_STRVAL_P(member));
		} else {
			debug("in %s ps22",__FUNCTION__);
			/* fall through, return property_info... */
		}
	} else {

		debug("in %s ps3",__FUNCTION__);
		EG(std_property_info).flags = ZEND_ACC_PUBLIC;
		EG(std_property_info).name = Z_STRVAL_P(member);
		EG(std_property_info).name_length = Z_STRLEN_P(member);
		EG(std_property_info).h = h;
		EG(std_property_info).ce = ce;
		property_info = &EG(std_property_info);
	}
	return property_info;
}

#endif
