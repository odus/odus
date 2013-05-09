/*
  +----------------------------------------------------------------------+
  | See COPYING file for further copyright information                   |
  +----------------------------------------------------------------------+ 
  | Author: Oleg Grenrus <oleg.grenrus@dynamoid.com>                     |
  | See CREDITS for contributors                                         |
  +----------------------------------------------------------------------+ 
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php_odus.h"
#include "odwrapper.h"
#include "od_igbinary.h"

ZEND_EXTERN_MODULE_GLOBALS(odus);

/* Static Variables. */

/* Static strings. */
static struct hash_si od_static_strings_hash;	// For serialize
static int od_static_strings_count = 0;

static char* od_static_strings[] = {
		"id",
		"name",
		"version",
		"progress",
		"Road",
		"extraData"
		/* Add more static strings here.
		   Note: for compatible consideration, this table should be only appended. DON'T modify/delete any exsiting string. */
	};

static char* EMPTY_STRING="";

extern void normal_od_wrapper_serialize(od_igbinary_serialize_data* igsd, zval* obj, uint8_t is_root);

/* {{{ Serializing functions prototypes */

inline static int od_igbinary_serialize64(od_igbinary_serialize_data *igsd, uint64_t i TSRMLS_DC);

inline static int od_igbinary_serialize_null(od_igbinary_serialize_data *igsd TSRMLS_DC);
inline static int od_igbinary_serialize_bool(od_igbinary_serialize_data *igsd, int b TSRMLS_DC);


inline static int od_igbinary_serialize_double(od_igbinary_serialize_data *igsd, double d TSRMLS_DC);

inline static int od_igbinary_serialize_chararray(od_igbinary_serialize_data *igsd, const char *s, uint32_t len TSRMLS_DC);

inline static int od_igbinary_serialize_chararray_ex(od_igbinary_serialize_data *igsd, const char *s, uint32_t len TSRMLS_DC);

inline static int od_igbinary_unserialize_string_from_table(od_igbinary_unserialize_data *igsd, char **s, uint32_t *len, uint32_t pos TSRMLS_DC);

inline static int od_igbinary_serialize_array_ref(od_igbinary_serialize_data *igsd, zval *z, bool object TSRMLS_DC);
inline static int od_igbinary_serialize_array_sleep(od_igbinary_serialize_data *igsd, zval *z, HashTable *ht, zend_class_entry *ce, bool incomplete_class TSRMLS_DC);
inline static int od_igbinary_serialize_object_name(od_igbinary_serialize_data *igsd, const char *name, uint32_t name_len TSRMLS_DC);
inline static int od_igbinary_serialize_object(od_igbinary_serialize_data *igsd, zval *z TSRMLS_DC);

/* }}} */
/* {{{ Unserializing functions prototypes */
inline static int od_igbinary_unserialize_string(od_igbinary_unserialize_data *igsd, od_igbinary_type t, char **s, uint32_t *len TSRMLS_DC);

inline static int od_igbinary_unserialize_array(od_igbinary_unserialize_data *igsd, od_igbinary_type t, zval **z, int object TSRMLS_DC);
inline static int od_igbinary_unserialize_object(od_igbinary_unserialize_data *igsd, od_igbinary_type t, zval **z TSRMLS_DC);
inline static int od_igbinary_unserialize_object_ser(od_igbinary_unserialize_data *igsd, od_igbinary_type t, zval **z, zend_class_entry *ce TSRMLS_DC);

//odus doesn't allow object references
//inline static int od_igbinary_unserialize_ref(od_igbinary_unserialize_data *igsd, od_igbinary_type t, zval **z TSRMLS_DC);

/* }}} */

inline static void adjust_len_info(od_igbinary_serialize_data *igsd, uint32_t n, uint32_t new_n, uint old_len_bytes, uint new_len_bytes, uint32_t old_len_pos);

inline int od_igbinary_init(TSRMLS_D) {
	int i;
	int len;

	od_static_strings_count = sizeof(od_static_strings) / sizeof(char*);
	
	if (!od_static_strings) {
		od_error(E_ERROR, "od_igbinary_init: fail to alloc memory");
		return 1;
	}

	hash_si_init(&od_static_strings_hash, 16);
	for (i = 0; i < od_static_strings_count; i++) {
		len = strlen(od_static_strings[i]);

		hash_si_insert(&od_static_strings_hash, od_static_strings[i], len, i);
	}

	return 0;
}

inline int od_igbinary_shutdown(TSRMLS_D) {
	hash_si_deinit(&od_static_strings_hash);

	return 0;
}

/* {{{ int od_igbinary_serialize(uint8_t**, uint32_t*, zval*) */
int od_igbinary_serialize(uint8_t **ret, uint32_t *ret_len, zval *z TSRMLS_DC) {
	od_igbinary_serialize_data igsd;

	if (od_igbinary_serialize_data_init(&igsd, Z_TYPE_P(z) != IS_OBJECT && Z_TYPE_P(z) != IS_ARRAY TSRMLS_CC)) {
		od_error(E_ERROR, "od_igbinary_serialize: cannot init igsd");
		return 1;
	}

	if (od_igbinary_serialize_header(&igsd TSRMLS_CC) != 0) {
		od_igbinary_serialize_data_deinit(&igsd TSRMLS_CC);
		return 1;
	}

	if (od_igbinary_serialize_zval(&igsd, z TSRMLS_CC) != 0) {
		od_igbinary_serialize_data_deinit(&igsd TSRMLS_CC);
		return 1;
	}

	if (igsd.compact_strings) {
		if (od_igbinary_serialize_string_table(&igsd TSRMLS_CC) != 0) {
			od_igbinary_serialize_data_deinit(&igsd TSRMLS_CC);
			return 1;
		}
	}

	*ret_len = igsd.buffer_size;

	/* Explicit nul termination */
	if(od_igbinary_serialize8(&igsd,0)!=0)
	{
		od_igbinary_serialize_data_deinit(&igsd TSRMLS_CC);
		return 1;
	}

	*ret = igsd.buffer;

	igsd.buffer = NULL;

	od_igbinary_serialize_data_deinit(&igsd TSRMLS_CC);

	return 0;
}
/* }}} */
/* {{{ int od_igbinary_unserialize(const uint8_t *, uint32_t, zval **) */
int od_igbinary_unserialize(const uint8_t *buf, uint32_t buf_len, zval **z TSRMLS_DC) {
	od_igbinary_unserialize_data igsd;
	uint32_t version = -1;

	od_igbinary_unserialize_data_init(&igsd TSRMLS_CC);

	igsd.buffer = (uint8_t *) buf;
	igsd.buffer_size = buf_len;

	if (od_igbinary_unserialize_header(&igsd, &version TSRMLS_CC)) {
		od_igbinary_unserialize_data_deinit(&igsd TSRMLS_CC);
		return 1;
	}

	if (igsd.compact_strings) {
		if (od_igbinary_unserialize_init_string_table(&igsd TSRMLS_CC)) {
			od_igbinary_unserialize_data_deinit(&igsd TSRMLS_CC);
			return 1;
		}
	}

	if (od_igbinary_unserialize_zval(&igsd, z TSRMLS_CC)) {
		od_igbinary_unserialize_data_deinit(&igsd TSRMLS_CC);
		return 1;
	}

	od_igbinary_unserialize_data_deinit(&igsd TSRMLS_CC);

	return 0;
}
/* }}} */
/* {{{ proto string od_igbinary_unserialize(mixed value) */
PHP_FUNCTION(od_unserialize) {
	char *string;
	int string_len;

	(void) return_value_ptr;
	(void) this_ptr;
	(void) return_value_used;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &string, &string_len) == FAILURE) {
		RETURN_NULL();
	}

	if (string_len <= 0) {
		RETURN_NULL();
	}

	if (od_igbinary_unserialize((uint8_t *) string, string_len, &return_value TSRMLS_CC)) {
		RETURN_NULL();
	}
}
/* }}} */

/* {{{ od_igbinary_serialize_data_init */
/** Inits od_igbinary_serialize_data. */
inline int od_igbinary_serialize_data_init(od_igbinary_serialize_data *igsd, bool scalar TSRMLS_DC) {
	int r = 0;

	igsd->buffer = NULL;
	igsd->buffer_size = 0;
	//igsd->buffer_capacity = 32;
	igsd->buffer_capacity = OD_RESERVED_BUFFER_LEN;

	igsd->buffer = (uint8_t *) emalloc(igsd->buffer_capacity);
	if (igsd->buffer == NULL) {
		return 1;
	}

	igsd->scalar = scalar;
	if (!igsd->scalar) {
		hash_si_init(&igsd->strings, 16);
		hash_si_init(&igsd->objects, 16);
	}
	igsd->compact_strings = (bool)ODUS_G(compact_strings);

	igsd->strings_count = 0;
	igsd->string_table_update = false;

	igsd->compress_value_len = true;

	igsd->root_id = 0;
	return r;
}
/* }}} */
/* {{{ od_igbinary_serialize_data_deinit */
/** Deinits od_igbinary_serialize_data. */
inline void od_igbinary_serialize_data_deinit(od_igbinary_serialize_data *igsd TSRMLS_DC) {
	if (igsd->buffer) {
		efree(igsd->buffer);
	}

	if (!igsd->scalar) {
		hash_si_deinit(&igsd->strings);
		hash_si_deinit(&igsd->objects);
	}
}
/* }}} */

/* {{{ od_igbinary_serialize_resize */
/** Expandes od_igbinary_serialize_data. */
inline static int od_igbinary_serialize_resize(od_igbinary_serialize_data *igsd, uint32_t size TSRMLS_DC) {
	if (igsd->buffer_size + size < igsd->buffer_capacity) {
		return 0;
	}

	while (igsd->buffer_size + size >= igsd->buffer_capacity) {
		igsd->buffer_capacity *= 2;
	}

	igsd->buffer = (uint8_t *) erealloc(igsd->buffer, igsd->buffer_capacity);
	if (igsd->buffer == NULL)
		return 1;

	return 0;
}
/* }}} */

/* {{{ od_igbinary_serialize_header */
/** Serializes header. */
inline int od_igbinary_serialize_header(od_igbinary_serialize_data *igsd TSRMLS_DC) {
	od_igbinary_serialize32(igsd, OD_IGBINARY_FORMAT_VERSION TSRMLS_CC); /* version */

	if (igsd->compact_strings) {
		// Jump over the string table info.
		if(od_igbinary_serialize_resize(igsd, OD_IGBINARY_STRING_TABLE_INFO_LEN)) {
			return 1;
		}
		igsd->buffer_size += OD_IGBINARY_STRING_TABLE_INFO_LEN;
	}

	return 0;
}
/* }}} */

inline void od_igbinary_serialize_append_zero(od_igbinary_serialize_data *igsd) {
	if(igsd->buffer_size >= igsd->buffer_capacity) {
		igsd->buffer = (uint8_t *) erealloc(igsd->buffer, igsd->buffer_size+1);
		igsd->buffer_capacity = igsd->buffer_size+1;
	}

	igsd->buffer[igsd->buffer_size]=0;
}

inline int od_igbinary_serialize_memcpy(od_igbinary_serialize_data *igsd, uint8_t* s, uint32_t len) {
	if(s == NULL || len == 0) return 1;

	if(od_igbinary_serialize_resize(igsd,len)) {
		return 1;
	}

	memcpy(igsd->buffer + igsd->buffer_size, s, len);

	igsd->buffer_size += len;

	return 0;
}


/* {{{ od_igbinary_serialize8 */
/** Serialize 8bit value. */
inline int od_igbinary_serialize8(od_igbinary_serialize_data *igsd, uint8_t i TSRMLS_DC) {
	if (od_igbinary_serialize_resize(igsd, 1 TSRMLS_CC)) {
		return 1;
	}

	igsd->buffer[igsd->buffer_size++] = i;
	return 0;
}
/* }}} */

/* {{{ od_igbinary_serialize8_at */
/** Serialize 8bit value at specified position.. */
inline int od_igbinary_serialize8_at(od_igbinary_serialize_data *igsd, uint8_t i, uint32_t pos TSRMLS_DC) {
	igsd->buffer[pos++] = i;
	return 0;
}
/* }}} */

/* {{{ od_igbinary_serialize16 */
/** Serialize 16bit value. */
inline int od_igbinary_serialize16(od_igbinary_serialize_data *igsd, uint16_t i TSRMLS_DC) {
	if (od_igbinary_serialize_resize(igsd, 2 TSRMLS_CC)) {
		return 1;
	}

	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 8 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i & 0xff);

	return 0;
}
/* }}} */

/* {{{ od_igbinary_serialize16_at */
/** Serialize 32bit value at specified position. */
inline int od_igbinary_serialize16_at(od_igbinary_serialize_data *igsd, uint16_t i, uint32_t pos TSRMLS_DC) {
	igsd->buffer[pos++] = (uint8_t) (i >> 8 & 0xff);
	igsd->buffer[pos++] = (uint8_t) (i & 0xff);

	return 0;
}
/* }}} */

/* {{{ od_igbinary_serialize32 */
/** Serialize 32bit value. */
inline int od_igbinary_serialize32(od_igbinary_serialize_data *igsd, uint32_t i TSRMLS_DC) {
	if (od_igbinary_serialize_resize(igsd, 4 TSRMLS_CC)) {
		return 1;
	}

	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 24 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 16 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 8 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i & 0xff);

	return 0;
}
/* }}} */

inline int od_igbinary_serialize_skip_n(od_igbinary_serialize_data *igsd, int n TSRMLS_DC) {
	if (od_igbinary_serialize_resize(igsd, n TSRMLS_CC)) {
		return 1;
	}

	igsd->buffer_size += n;

	return 0;
}

/* {{{ od_igbinary_serialize32_at */
/** Serialize 32bit value at specified position. */
inline int od_igbinary_serialize32_at(od_igbinary_serialize_data *igsd, uint32_t i, uint32_t pos TSRMLS_DC) {
	igsd->buffer[pos++] = (uint8_t) (i >> 24 & 0xff);
	igsd->buffer[pos++] = (uint8_t) (i >> 16 & 0xff);
	igsd->buffer[pos++] = (uint8_t) (i >> 8 & 0xff);
	igsd->buffer[pos++] = (uint8_t) (i & 0xff);

	return 0;
}
/* }}} */

/* {{{ od_igbinary_serialize_value_len */
/** Serialize 32bit value. */
inline int od_igbinary_serialize_value_len(od_igbinary_serialize_data *igsd, uint32_t len, uint32_t pos TSRMLS_DC) {
	if (igsd->compress_value_len) {
		/** Compress size, use the first two bits to indicate length:
		 * 01: Len will occupy 1 byte;
		 * 10: Len will occupy 2 bytes in total;
		 * 11: Not used for now;
		 * 00: Len will occupy 4 bytes in total. This will be compatible with the old format (always be 4 bytes.) */
		uint32_t encode_len = 0;
		if (len <= 0x3f) {
			encode_len = len | 0x40;
			od_igbinary_serialize8_at(igsd, (uint8_t) encode_len, pos TSRMLS_CC);
			memmove(igsd->buffer + pos + 1, igsd->buffer + pos + OD_IGBINARY_VALUE_LEN_SIZE, igsd->buffer_size - OD_IGBINARY_VALUE_LEN_SIZE - pos);
			igsd->buffer_size -= 3;
		} else if (len <= 0x3fff) {
			encode_len = len | 0x8000;
			od_igbinary_serialize16_at(igsd, (uint16_t) encode_len, pos TSRMLS_CC);
			memmove(igsd->buffer + pos + 2, igsd->buffer + pos + OD_IGBINARY_VALUE_LEN_SIZE, igsd->buffer_size - OD_IGBINARY_VALUE_LEN_SIZE - pos);
			igsd->buffer_size -= 2;
		} else if (len <= 0x3fffffff) {
			encode_len = len;
			od_igbinary_serialize32_at(igsd, (uint32_t) encode_len, pos TSRMLS_CC);
		} else {
			od_error(E_ERROR, "od_igbinary_serialize_value_len: len is too large");
			return 1;
		}
		return 0;
	} else {
		return od_igbinary_serialize32_at(igsd, len, pos TSRMLS_CC);
	}
}
/* }}} */

/* {{{ od_igbinary_serialize64 */
/** Serialize 64bit value. */
inline static int od_igbinary_serialize64(od_igbinary_serialize_data *igsd, uint64_t i TSRMLS_DC) {
	if (od_igbinary_serialize_resize(igsd, 8 TSRMLS_CC)) {
		return 1;
	}

	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 56 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 48 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 40 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 32 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 24 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 16 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 8 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i & 0xff);

	return 0;
}
/* }}} */
/* {{{ od_igbinary_serialize_null */
/** Serializes null. */
inline static int od_igbinary_serialize_null(od_igbinary_serialize_data *igsd TSRMLS_DC) {
	return od_igbinary_serialize8(igsd, od_igbinary_type_null TSRMLS_CC);
}
/* }}} */
/* {{{ od_igbinary_serialize_bool */
/** Serializes bool. */
inline static int od_igbinary_serialize_bool(od_igbinary_serialize_data *igsd, int b TSRMLS_DC) {
	return od_igbinary_serialize8(igsd, (uint8_t) (b ? od_igbinary_type_bool_true : od_igbinary_type_bool_false) TSRMLS_CC);
}
/* }}} */
/* {{{ od_igbinary_serialize_long */
/** Serializes long. */
inline int od_igbinary_serialize_long(od_igbinary_serialize_data *igsd, long l TSRMLS_DC) {
	long k = l >= 0 ? l : -l;
	bool p = l >= 0 ? true : false;

	/* -LONG_MIN is 0 otherwise. */
	if (l == LONG_MIN) {
#if SIZEOF_LONG == 8
		od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_long64n TSRMLS_CC);
		od_igbinary_serialize64(igsd, (uint64_t) 0x8000000000000000 TSRMLS_CC);
#elif SIZEOF_LONG == 4
		od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_long32n TSRMLS_CC);
		od_igbinary_serialize32(igsd, (uint32_t) 0x80000000 TSRMLS_CC);
#else
#error "Strange sizeof(long)."
#endif
		return 0;
	}

	if (k <= 0xff) {
		od_igbinary_serialize8(igsd, (uint8_t) (p ? od_igbinary_type_long8p : od_igbinary_type_long8n) TSRMLS_CC);
		od_igbinary_serialize8(igsd, (uint8_t) k TSRMLS_CC);
	} else if (k <= 0xffff) {
		od_igbinary_serialize8(igsd, (uint8_t) (p ? od_igbinary_type_long16p : od_igbinary_type_long16n) TSRMLS_CC);
		od_igbinary_serialize16(igsd, (uint16_t) k TSRMLS_CC);
#if SIZEOF_LONG == 8
	} else if (k <= 0xffffffff) {
		od_igbinary_serialize8(igsd, (uint8_t) (p ? od_igbinary_type_long32p : od_igbinary_type_long32n) TSRMLS_CC);
		od_igbinary_serialize32(igsd, (uint32_t) k TSRMLS_CC);
	} else {
		od_igbinary_serialize8(igsd, (uint8_t) (p ? od_igbinary_type_long64p : od_igbinary_type_long64n) TSRMLS_CC);
		od_igbinary_serialize64(igsd, (uint64_t) k TSRMLS_CC);
	}
#elif SIZEOF_LONG == 4
	} else {
		od_igbinary_serialize8(igsd, (uint8_t) (p ? od_igbinary_type_long32p : od_igbinary_type_long32n) TSRMLS_CC);
		od_igbinary_serialize32(igsd, (uint32_t) k TSRMLS_CC);
	}
#else
#error "Strange sizeof(long)."
#endif

	return 0;
}
/* }}} */
/* {{{ od_igbinary_serialize_double */
/** Serializes double. */
inline static int od_igbinary_serialize_double(od_igbinary_serialize_data *igsd, double d TSRMLS_DC) {
	union {
		double d;
		uint64_t u;
	} u;

	od_igbinary_serialize8(igsd, od_igbinary_type_double TSRMLS_CC);

	u.d = d;

	od_igbinary_serialize64(igsd, u.u TSRMLS_CC);

	return 0;
}
/* }}} */
/* {{{ od_igbinary_serialize_string */
/** Serializes string.
 * Serializes each string once, after first time uses pointers.
 */
inline int od_igbinary_serialize_string(od_igbinary_serialize_data *igsd, const char *s, uint32_t len TSRMLS_DC) {
	uint32_t t;
	uint32_t *i = &t;

	if (len == 0) {
		od_igbinary_serialize8(igsd, od_igbinary_type_string_empty TSRMLS_CC);
		return 0;
	}

	if (igsd->scalar || !igsd->compact_strings) {
		if (od_igbinary_serialize_chararray(igsd, s, len TSRMLS_CC) != 0) {
			return 1;
		}
	} else {	/* !igsd->scalar && igsd->compact_strings */
		if (hash_si_find(&igsd->strings, s, len, i) == 1) {
			hash_si_insert(&igsd->strings, s, len, igsd->strings_count);
			*i = igsd->strings_count;
			igsd->strings_count += 1;
		}

		if (*i <= 0xff) {
			od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_string_id8 TSRMLS_CC);
			od_igbinary_serialize8(igsd, (uint8_t) *i TSRMLS_CC);
		} else if (*i <= 0xffff) {
			od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_string_id16 TSRMLS_CC);
			od_igbinary_serialize16(igsd, (uint16_t) *i TSRMLS_CC);
		} else {
			od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_string_id32 TSRMLS_CC);
			od_igbinary_serialize32(igsd, (uint32_t) *i TSRMLS_CC);
		}
	}

	return 0;
}
/* }}} */
/* {{{ od_igbinary_serialize_chararray */
/** Serializes string data. */
inline static int od_igbinary_serialize_chararray(od_igbinary_serialize_data *igsd, const char *s, uint32_t len TSRMLS_DC) {

	if (len <= 0xff) {
		od_igbinary_serialize8(igsd, od_igbinary_type_string8 TSRMLS_CC);
		od_igbinary_serialize8(igsd, len TSRMLS_CC);
	} else if (len <= 0xffff) {
		od_igbinary_serialize8(igsd, od_igbinary_type_string16 TSRMLS_CC);
		od_igbinary_serialize16(igsd, len TSRMLS_CC);
	} else {
		od_igbinary_serialize8(igsd, od_igbinary_type_string32 TSRMLS_CC);
		od_igbinary_serialize32(igsd, len TSRMLS_CC);
	}

	if (od_igbinary_serialize_resize(igsd, len +1 TSRMLS_CC)) {
		return 1;
	}

	memcpy(igsd->buffer+igsd->buffer_size, s, len);
	igsd->buffer_size += len;

	//XXX
	// odus need '\0' at the end of string for lazy reading
	igsd->buffer[igsd->buffer_size++]='\0';

	return 0;
}
/* }}} */

/* {{{ od_igbinary_serialize_chararray_ex */
/** Serializes string data, enhanced to work with static string table. */
inline static int od_igbinary_serialize_chararray_ex(od_igbinary_serialize_data *igsd, const char *s, uint32_t len TSRMLS_DC) {
	uint32_t t;
	uint32_t *i = &t;

	if (hash_si_find(&od_static_strings_hash, s, len, i) == 0) {
		if (*i <= 0xff) {
			od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_static_string_id8 TSRMLS_CC);
			od_igbinary_serialize8(igsd, (uint8_t) *i TSRMLS_CC);
		} else if (*i <= 0xffff) {
			od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_static_string_id16 TSRMLS_CC);
			od_igbinary_serialize16(igsd, (uint16_t) *i TSRMLS_CC);
		} else {
			od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_static_string_id32 TSRMLS_CC);
			od_igbinary_serialize32(igsd, (uint32_t) *i TSRMLS_CC);
		}
		return 0;
	} else {
		return od_igbinary_serialize_chararray(igsd, s, len TSRMLS_CC);
	}	
}
/* }}} */

/* {{{ od_igbinary_serialize_array_key */
/** Serializes a key of array. */
inline static int od_igbinary_serialize_array_key(od_igbinary_serialize_data *igsd, int object, int key_type, ulong key_index, char *key, uint key_len TSRMLS_DC) {
	if (key_type==HASH_KEY_IS_LONG) {
		return od_igbinary_serialize_long(igsd, key_index TSRMLS_CC);
	} else if (object) {
		// We enable string compact only for object properties name.
		return od_igbinary_serialize_string(igsd, key, key_len TSRMLS_CC);
	} else {
		return od_igbinary_serialize_chararray_ex(igsd, key, key_len TSRMLS_CC);
	}
}
/* }}} */


/* {{{ igbinay_serialize_array */
/** Serializes array or objects inner properties. */
inline int od_igbinary_serialize_array(od_igbinary_serialize_data *igsd, zval *z, zend_class_entry* ce, bool object, bool incomplete_class, bool in_od_serialize TSRMLS_DC) {
	HashTable *h;
	HashPosition pos;
	uint32_t n;
	zval **d;

	char *key;
	uint key_len;
	int key_type;
	ulong key_index;

	int r = 0;

	/* hash */
	h = object ? Z_OBJPROP_P(z) : HASH_OF(z);

	/* hash size */
	n = h ? zend_hash_num_elements(h) : 0;

	/* incomplete class magic member */
	if (n > 0 && incomplete_class) {
		--n;
	}

	if (!object && od_igbinary_serialize_array_ref(igsd, z, object TSRMLS_CC) == 0) {
			return 1; // if we see that we serialized a ref then return error in ODUS 
	}

	//for removiing default properties
	uint num_defaults = 0;
	uint old_len_bytes;
	uint32_t old_len_pos = igsd->buffer_size;

	if (n <= 0xff) {
		od_igbinary_serialize8(igsd, od_igbinary_type_array8 TSRMLS_CC);
		od_igbinary_serialize8(igsd, n TSRMLS_CC);
	} else if (n <= 0xffff) {
		od_igbinary_serialize8(igsd, od_igbinary_type_array16 TSRMLS_CC);
		od_igbinary_serialize16(igsd, n TSRMLS_CC);
	} else {
		od_igbinary_serialize8(igsd, od_igbinary_type_array32 TSRMLS_CC);
		od_igbinary_serialize32(igsd, n TSRMLS_CC);
	}

	old_len_bytes = igsd->buffer_size - old_len_pos;

	//XXX
	// odus will save the length of array at the buffer now
	if (od_igbinary_serialize_resize(igsd, OD_IGBINARY_VALUE_LEN_SIZE TSRMLS_CC)) {
		return 1;
	}

	if (n == 0) {
		igsd->buffer_size += OD_IGBINARY_VALUE_LEN_SIZE;
		od_igbinary_serialize_value_len(igsd,0,igsd->buffer_size - OD_IGBINARY_VALUE_LEN_SIZE);
		return 0;
	}

	igsd->buffer_size += OD_IGBINARY_VALUE_LEN_SIZE;
	uint32_t value_start = igsd->buffer_size;

	/* serialize properties. */
	zend_hash_internal_pointer_reset_ex(h, &pos);
	for (;; zend_hash_move_forward_ex(h, &pos)) {
		key_type = zend_hash_get_current_key_ex(h, &key, &key_len, &key_index, 0, &pos);

		/* last */
		if (key_type == HASH_KEY_NON_EXISTANT) {
			break;
		}

		/* skip magic member in incomplete classes */
		if (incomplete_class && strcmp(key, MAGIC_MEMBER) == 0) {
			continue;
		}

		if(key_type != HASH_KEY_IS_LONG && key_type != HASH_KEY_IS_STRING) {
			od_error(E_ERROR, "od_igbinary_serialize_array: key is not string nor long");
			/* not reached */
			return 1;
		}

		/* we should still add element even if it's not OK,
		 * since we already wrote the length of the array before */
		if (zend_hash_get_current_data_ex(h, (void *) &d, &pos) != SUCCESS || d == NULL) {
			r += od_igbinary_serialize_array_key(igsd, object, key_type, key_index, key, key_len - 1 TSRMLS_CC);

			if (od_igbinary_serialize_null(igsd TSRMLS_CC)) {
				return 1;
			}
		} else {
			if(in_od_serialize) {
				r += od_igbinary_serialize_array_key(igsd, object, key_type, key_index, key, key_len - 1 TSRMLS_CC);

				normal_od_wrapper_serialize(igsd, *d, 0);
			} else {

				if(object && ODUS_G(remove_default) && !incomplete_class && ce && pos && is_default(pos->arKey,pos->nKeyLength,pos->h,*d,&ce->default_properties)) {
					num_defaults ++;
				} else {
					r += od_igbinary_serialize_array_key(igsd, object, key_type, key_index, key, key_len - 1 TSRMLS_CC);

					if (od_igbinary_serialize_zval(igsd, *d TSRMLS_CC)) {
						return 1;
					}
				}

			}
		}

	}

	uint32_t len = igsd->buffer_size - value_start;

	od_igbinary_serialize_value_len(igsd,len,value_start-OD_IGBINARY_VALUE_LEN_SIZE);

	if(num_defaults > 0) {

		uint32_t new_n = (num_defaults<=n)?(n-num_defaults):0;
		uint new_len_bytes = (new_n <= 0xff)? 2 : ((new_n <= 0xffff)? 3 : 5);

		adjust_len_info(igsd, n, new_n, old_len_bytes, new_len_bytes, old_len_pos);
	}

	return r;
}
/* }}} */

/* {{{ od_igbinary_serialize_string_table */
/** Serializes the string table. */
inline int od_igbinary_serialize_string_table(od_igbinary_serialize_data *igsd TSRMLS_DC) {
	int i = 0;
	struct hash_si *h = &igsd->strings;
	uint32_t *indexes = (int*)emalloc(sizeof(int) * igsd->strings_count);
	uint32_t string_table_start = igsd->buffer_size;

	if (!indexes) {
		return 1;
	}

	od_igbinary_serialize32_at(igsd, string_table_start, OD_IGBINARY_STRING_TABLE_START_OFFSET	TSRMLS_CC);

	// strings
	for (i = 0; i < h->size; i++) {
		if (h->data[i].key != NULL) {
			indexes[h->data[i].value] = igsd->buffer_size - string_table_start;
			od_igbinary_serialize_chararray(igsd, h->data[i].key, h->data[i].key_len TSRMLS_CC);
		}
	}

	od_igbinary_serialize32(igsd, igsd->strings_count TSRMLS_CC);

	od_igbinary_serialize32_at(igsd, igsd->buffer_size, OD_IGBINARY_STRING_TABLE_INDEX_OFFSET	TSRMLS_CC);

	// indexes
	for (i = 0; i < igsd->strings_count; i++) {
		od_igbinary_serialize32(igsd, indexes[i] TSRMLS_CC);
	}

	efree(indexes);

	return 0;
}
/* }}} */

inline int od_igbinary_clone_string_table(od_igbinary_serialize_data *igsd, od_igbinary_unserialize_data *orig TSRMLS_DC) {
	int32_t string_table_start = 0;
	int32_t index_offset = 0;
	uint32_t strings_count = 0;
	int32_t i = 0;
	char *str = NULL;
	uint32_t str_len = 0;
	uint32_t string_offset = 0;

	char *buffer_backup = orig->buffer;
	orig->buffer = orig->original_buffer;
	string_table_start = (int32_t)od_igbinary_unserialize32_at(orig, OD_IGBINARY_STRING_TABLE_START_OFFSET	TSRMLS_CC);
	index_offset = (int32_t)od_igbinary_unserialize32_at(orig, OD_IGBINARY_STRING_TABLE_INDEX_OFFSET	TSRMLS_CC);
	strings_count = od_igbinary_unserialize32_at(orig, index_offset - 4	TSRMLS_CC);

	for(i = 0; i < strings_count; i++) {
		string_offset = string_table_start + od_igbinary_unserialize32_at(orig, index_offset + 4 * i	TSRMLS_CC);
		od_igbinary_unserialize_string_from_table(orig, &str, &str_len, string_offset TSRMLS_CC);
		hash_si_insert(&igsd->strings, str, str_len, i);
	}
	igsd->strings_count = strings_count;

	orig->buffer = buffer_backup;
	return 0;
}

/* {{{ od_igbinary_serialize_update_string_table */
/** Update offset for the string table. */
inline int od_igbinary_serialize_update_string_table(od_igbinary_serialize_data *igsd, od_igbinary_unserialize_data *orig, int32_t delta TSRMLS_DC) {
	int32_t string_table_start = 0;
	int32_t index_offset = 0;
	uint32_t original_strings_count = 0;

	char *buffer_backup = orig->buffer;
	orig->buffer = orig->original_buffer;
	string_table_start = (int32_t)od_igbinary_unserialize32_at(orig, OD_IGBINARY_STRING_TABLE_START_OFFSET	TSRMLS_CC);
	index_offset = (int32_t)od_igbinary_unserialize32_at(orig, OD_IGBINARY_STRING_TABLE_INDEX_OFFSET	TSRMLS_CC);
	original_strings_count = od_igbinary_unserialize32_at(orig, index_offset - 4	TSRMLS_CC);

	if (igsd->string_table_update) {
		od_igbinary_serialize_string_table(igsd TSRMLS_CC);
	} else {
		// Only update the offset.
		od_igbinary_serialize_memcpy(igsd, orig->buffer + string_table_start, index_offset + original_strings_count * 4 - string_table_start);
		od_igbinary_serialize32_at(igsd, (uint32_t)(string_table_start + delta), OD_IGBINARY_STRING_TABLE_START_OFFSET	TSRMLS_CC);
		od_igbinary_serialize32_at(igsd, (uint32_t)(index_offset + delta), OD_IGBINARY_STRING_TABLE_INDEX_OFFSET	TSRMLS_CC);
	}

	orig->buffer = buffer_backup;
	return 0;
}
/* }}} */

/* {{{ od_igbinary_serialize_array_ref */
/** Serializes array reference. */

// Will always return 1 or trigger fatal error
inline static int od_igbinary_serialize_array_ref(od_igbinary_serialize_data *igsd, zval *z, bool object TSRMLS_DC) {
	uint32_t t = 0;
	uint32_t *i = &t;
	union {
		zval *z;
		struct {
			zend_class_entry *ce;
			zend_object_handle handle;
		} obj;
	} key = { 0 };

	if (object && Z_TYPE_P(z) == IS_OBJECT && Z_OBJ_HT_P(z)->get_class_entry) {
		key.obj.ce = Z_OBJCE_P(z);
		key.obj.handle = Z_OBJ_HANDLE_P(z);
	} else {
		key.z = z;
	}

	if ( (object || Z_ISREF_P(z)) && hash_si_find(&igsd->objects, (char *)&key, sizeof(key), i) == 1) {
		t = hash_si_size(&igsd->objects);
		hash_si_insert(&igsd->objects, (char *)&key, sizeof(key), t);
		return 1;
	} else {

		if(!object && !Z_ISREF_P(z)) {
			//XXX
			// no referred array is valid in odus
			return 1;
		}

		// Depending on what behavior you want out of the extension you can enable this 
		// in /etc/php.d/odus.ini ( set odus.throw_exceptions=1 )
		if(ODUS_G(od_throw_exceptions)) {
				zend_throw_exception(odus_exception_ce, "Odus detected reference to OBJECT/ARRAY in data which is not supported by odus serialization", -1 TSRMLS_CC);
		}

		// Turn reduce php fatals 
		// in /etc/php.d/odus.ini ( set odus.reduce_fatals=1 )
		if(!ODUS_G(od_reduce_fatals)) { // default behavor of ODUS is to have reduce fatals off
				od_error(E_ERROR, "in odus different values could not referring to same OBJECT or ARRAY");
		}

		od_igbinary_type type;
		if (*i <= 0xff) {
			type = object ? od_igbinary_type_objref8 : od_igbinary_type_ref8;
			od_igbinary_serialize8(igsd, (uint8_t) type TSRMLS_CC);
			od_igbinary_serialize8(igsd, (uint8_t) *i TSRMLS_CC);
		} else if (*i <= 0xffff) {
			type = object ? od_igbinary_type_objref16 : od_igbinary_type_ref16;
			od_igbinary_serialize8(igsd, (uint8_t) type TSRMLS_CC);
			od_igbinary_serialize16(igsd, (uint16_t) *i TSRMLS_CC);
		} else {
			type = object ? od_igbinary_type_objref32 : od_igbinary_type_ref32;
			od_igbinary_serialize8(igsd, (uint8_t) type TSRMLS_CC);
			od_igbinary_serialize32(igsd, (uint32_t) *i TSRMLS_CC);
		}

		return 0;
	}

	return 1;
}
/* }}} */

inline static void adjust_len_info(od_igbinary_serialize_data *igsd, uint32_t n, uint32_t new_n, uint old_len_bytes, uint new_len_bytes, uint32_t old_len_pos) {

	assert(old_len_bytes >= new_len_bytes);

	debug("%u properties are defaults and removed out",n - new_n);

	uint32_t diff = old_len_bytes - new_len_bytes;

	uint32_t real_size = igsd->buffer_size - diff;

	if(diff>0) {
		uint32_t pos;

		for(pos = old_len_pos + new_len_bytes; pos < real_size; pos++) {
			igsd->buffer[pos] = igsd->buffer[pos + diff];
		}
	}

	igsd->buffer_size = old_len_pos;

	if (new_n <= 0xff) {
		igsd->buffer[igsd->buffer_size++] = (uint8_t)od_igbinary_type_array8;
		igsd->buffer[igsd->buffer_size++] = (uint8_t)new_n;

	} else if (new_n <= 0xffff) {

		igsd->buffer[igsd->buffer_size++] = (uint8_t)od_igbinary_type_array16;

		igsd->buffer[igsd->buffer_size++] = (uint8_t) (new_n >> 8 & 0xff);
		igsd->buffer[igsd->buffer_size++] = (uint8_t) (new_n & 0xff);

	} else {
		igsd->buffer[igsd->buffer_size++] = (uint8_t)od_igbinary_type_array32;

		igsd->buffer[igsd->buffer_size++] = (uint8_t) (new_n >> 24 & 0xff);
		igsd->buffer[igsd->buffer_size++] = (uint8_t) (new_n >> 16 & 0xff);
		igsd->buffer[igsd->buffer_size++] = (uint8_t) (new_n >> 8 & 0xff);
		igsd->buffer[igsd->buffer_size++] = (uint8_t) (new_n & 0xff);
	}

	igsd->buffer_size = real_size;
}

/* {{{ od_igbinary_serialize_array_sleep */
/** Serializes object's properties array with __sleep -function. */
inline static int od_igbinary_serialize_array_sleep(od_igbinary_serialize_data *igsd, zval *z, HashTable *h, zend_class_entry *ce, bool incomplete_class TSRMLS_DC) {
	HashPosition pos;
	uint32_t n = zend_hash_num_elements(h);
	zval **d;
	zval **v;

	char *key;
	uint key_len;
	int key_type;
	ulong key_index;

	//for removiing default properties
	uint num_defaults = 0;
	uint old_len_bytes;
	ulong hash;
	uint32_t old_len_pos = igsd->buffer_size;

	int r = 0;

	/* Decrease array size by one, because of magic member (with class name) */
	if (n > 0 && incomplete_class) {
		--n;
	}

	/* Serialize array id. */
	if (n <= 0xff) {
		od_igbinary_serialize8(igsd, od_igbinary_type_array8 TSRMLS_CC);
		od_igbinary_serialize8(igsd, n TSRMLS_CC);
	} else if (n <= 0xffff) {
		od_igbinary_serialize8(igsd, od_igbinary_type_array16 TSRMLS_CC);
		od_igbinary_serialize16(igsd, n TSRMLS_CC);
	} else {
		od_igbinary_serialize8(igsd, od_igbinary_type_array32 TSRMLS_CC);
		od_igbinary_serialize32(igsd, n TSRMLS_CC);
	}

	old_len_bytes = igsd->buffer_size - old_len_pos;

	//XXX
	// odus will save the length of array at the buffer now
	if (od_igbinary_serialize_resize(igsd, OD_IGBINARY_VALUE_LEN_SIZE TSRMLS_CC)) {
		return 1;
	}

	if (n == 0) {
		igsd->buffer_size += OD_IGBINARY_VALUE_LEN_SIZE;
		od_igbinary_serialize_value_len(igsd,0,igsd->buffer_size - OD_IGBINARY_VALUE_LEN_SIZE);
		return 0;
	}

	igsd->buffer_size += OD_IGBINARY_VALUE_LEN_SIZE;
	uint32_t value_start = igsd->buffer_size;

	zend_hash_internal_pointer_reset_ex(h, &pos);

	for (;; zend_hash_move_forward_ex(h, &pos)) {
		key_type = zend_hash_get_current_key_ex(h, &key, &key_len, &key_index, 0, &pos);

		/* last */
		if (key_type == HASH_KEY_NON_EXISTANT) {
			break;
		}

		/* skip magic member in incomplete classes */
		if (incomplete_class && strcmp(key, MAGIC_MEMBER) == 0) {
			continue;
		}

		if (zend_hash_get_current_data_ex(h, (void *) &d, &pos) != SUCCESS || d == NULL || Z_TYPE_PP(d) != IS_STRING) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "__sleep should return an array only "
					"containing the names of instance-variables to "
					"serialize");

			/* we should still add element even if it's not OK,
			 * since we already wrote the length of the array before
			 * serialize null as key-value pair */
			r += od_igbinary_serialize_null(igsd TSRMLS_CC);
		} else {
			hash = zend_get_hash_value(Z_STRVAL_PP(d), Z_STRLEN_PP(d) + 1);
			if (zend_hash_quick_find(Z_OBJPROP_P(z), Z_STRVAL_PP(d), Z_STRLEN_PP(d) + 1, hash, (void *) &v) == SUCCESS) {
				if(ODUS_G(remove_default) && !incomplete_class && ce && is_default(Z_STRVAL_PP(d),Z_STRLEN_PP(d)+1,hash,*v,&ce->default_properties)) {
					num_defaults ++;
				} else {
					//r += od_igbinary_serialize_string(igsd, Z_STRVAL_PP(d), Z_STRLEN_PP(d) TSRMLS_CC);
					r += od_igbinary_serialize_array_key(igsd, true, HASH_KEY_IS_STRING, -1, Z_STRVAL_PP(d), Z_STRLEN_PP(d) TSRMLS_CC);
					r += od_igbinary_serialize_zval(igsd, *v TSRMLS_CC);
				}
			} else if (ce) {
				char *prot_name = NULL;
				char *priv_name = NULL;
				int prop_name_length;

				do {
					/* try private */
					zend_mangle_property_name(&priv_name, &prop_name_length, ce->name, ce->name_length,
								Z_STRVAL_PP(d), Z_STRLEN_PP(d), ce->type & ZEND_INTERNAL_CLASS);
					hash = zend_get_hash_value(priv_name, prop_name_length+1);
					if (zend_hash_quick_find(Z_OBJPROP_P(z), priv_name, prop_name_length+1, hash, (void *) &v) == SUCCESS) {
						if(ODUS_G(remove_default) && !incomplete_class && ce && is_default(priv_name,prop_name_length+1,hash,*v,&ce->default_properties)) {
							num_defaults ++;
						} else {
							//r += od_igbinary_serialize_string(igsd, priv_name, prop_name_length TSRMLS_CC);
							r += od_igbinary_serialize_array_key(igsd, true, HASH_KEY_IS_STRING, -1, priv_name, prop_name_length TSRMLS_CC);
							r += od_igbinary_serialize_zval(igsd, *v TSRMLS_CC);
						}

						efree(priv_name);
						break;
					}
					efree(priv_name);

					/* try protected */
					zend_mangle_property_name(&prot_name, &prop_name_length, "*", 1,
								Z_STRVAL_PP(d), Z_STRLEN_PP(d), ce->type & ZEND_INTERNAL_CLASS);
					hash = zend_get_hash_value(prot_name, prop_name_length+1);
					if (zend_hash_quick_find(Z_OBJPROP_P(z), prot_name, prop_name_length+1, hash, (void *) &v) == SUCCESS) {
						if(ODUS_G(remove_default) && !incomplete_class && ce && is_default(prot_name,prop_name_length+1,hash,*v,&ce->default_properties)) {
							num_defaults ++;
						} else {
							//r += od_igbinary_serialize_string(igsd, prot_name, prop_name_length TSRMLS_CC);
							r += od_igbinary_serialize_array_key(igsd, true, HASH_KEY_IS_STRING, -1, prot_name, prop_name_length TSRMLS_CC);
							r += od_igbinary_serialize_zval(igsd, *v TSRMLS_CC);
						}

						efree(prot_name);
						break;
					}
					efree(prot_name);

					/* no win */
					//r += od_igbinary_serialize_string(igsd, Z_STRVAL_PP(d), Z_STRLEN_PP(d) TSRMLS_CC);
					r += od_igbinary_serialize_array_key(igsd, true, HASH_KEY_IS_STRING, -1, Z_STRVAL_PP(d), Z_STRLEN_PP(d) TSRMLS_CC);
					r += od_igbinary_serialize_null(igsd TSRMLS_CC);
					php_error_docref(NULL TSRMLS_CC, E_NOTICE, "\"%s\" returned as member variable from __sleep() but does not exist", Z_STRVAL_PP(d));
				} while (0);

			} else {
				// if all else fails, just serialize the value in anyway.
				//r += od_igbinary_serialize_string(igsd, Z_STRVAL_PP(d), Z_STRLEN_PP(d) TSRMLS_CC);
				r += od_igbinary_serialize_array_key(igsd, true, HASH_KEY_IS_STRING, -1, Z_STRVAL_PP(d), Z_STRLEN_PP(d) TSRMLS_CC);
				r += od_igbinary_serialize_zval(igsd, *v TSRMLS_CC);
			}
		}
	}

	uint32_t len = igsd->buffer_size - value_start;

	od_igbinary_serialize_value_len(igsd,len,value_start-OD_IGBINARY_VALUE_LEN_SIZE);

	if(num_defaults > 0) {

		uint32_t new_n = (num_defaults<=n)?(n-num_defaults):0;
		uint new_len_bytes = (new_n <= 0xff)? 2 : ((new_n <= 0xffff)? 3 : 5);

		adjust_len_info(igsd, n, new_n, old_len_bytes, new_len_bytes, old_len_pos);
	}

	return r;
}
/* }}} */
/* {{{ od_igbinary_serialize_object_name */
/** Serialize object name. */
inline static int od_igbinary_serialize_object_name(od_igbinary_serialize_data *igsd, const char *class_name, uint32_t name_len TSRMLS_DC) {
	uint32_t t;
	uint32_t *i = &t;

	// Always insert object name into string table in ODUS 2.0
	if (hash_si_find(&igsd->strings, class_name, name_len, i) == 1) {
		hash_si_insert(&igsd->strings, class_name, name_len, igsd->strings_count);
		*i = igsd->strings_count;
		igsd->strings_count += 1;
	}

	if (*i <= 0xff) {
		od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_object_id8 TSRMLS_CC);
		od_igbinary_serialize8(igsd, (uint8_t) *i TSRMLS_CC);
	} else if (*i <= 0xffff) {
		od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_object_id16 TSRMLS_CC);
		od_igbinary_serialize16(igsd, (uint16_t) *i TSRMLS_CC);
	} else {
		od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_object_id32 TSRMLS_CC);
		od_igbinary_serialize32(igsd, (uint32_t) *i TSRMLS_CC);
	}

	return 0;
}
/* }}} */
/* {{{ od_igbinary_serialize_object */
/** Serialize object.
 * @see ext/standard/var.c
 * */
inline static int od_igbinary_serialize_object(od_igbinary_serialize_data *igsd, zval *z TSRMLS_DC) {
	zend_class_entry *ce;

	zval f;
	zval *h = NULL;

	int r = 0;

	unsigned char *serialized_data = NULL;
	zend_uint serialized_len;

	PHP_CLASS_ATTRIBUTES;

	if (od_igbinary_serialize_array_ref(igsd, z, true TSRMLS_CC) == 0) {
		return 1; // if we see that we serialized a ref then return error in ODUS 
	}

	ce = Z_OBJCE_P(z);

#if ALLOW_CLASS_SERIALIZE
	/* custom serializer */
	if (ce && ce->serialize != NULL) {
		//XXX
		// odus doesn't allow class has its serialize method
		od_error(E_ERROR,"class '%s' could not have its serialize method in odus", ce->name);

		/* TODO: var_hash? */
		if(ce->serialize(z, &serialized_data, &serialized_len, (zend_serialize_data *)NULL TSRMLS_CC) == SUCCESS && !EG(exception)) {
			od_igbinary_serialize_object_name(igsd, ce->name, ce->name_length TSRMLS_CC);

			if (serialized_len <= 0xff) {
				od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_object_ser8 TSRMLS_CC);
				od_igbinary_serialize8(igsd, (uint8_t) serialized_len TSRMLS_CC);
			} else if (serialized_len <= 0xffff) {
				od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_object_ser16 TSRMLS_CC);
				od_igbinary_serialize16(igsd, (uint16_t) serialized_len TSRMLS_CC);
			} else {
				od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_object_ser32 TSRMLS_CC);
				od_igbinary_serialize32(igsd, (uint32_t) serialized_len TSRMLS_CC);
			}

			if (od_igbinary_serialize_resize(igsd, serialized_len TSRMLS_CC)) {
				if (serialized_data) {
					efree(serialized_data);
				}
				r = 1;

				return r;
			}

			memcpy(igsd->buffer+igsd->buffer_size, serialized_data, serialized_len);
			igsd->buffer_size += serialized_len;
		} else if (EG(exception)) {
			/* exception, return failure */
			r = 1;
		} else {
			/* Serialization callback failed, assume null output */
			od_igbinary_serialize_null(igsd TSRMLS_CC);
		}

		if (serialized_data) {
			efree(serialized_data);
		}

		return r;
	}

#endif

	/* serialize class name */
	PHP_SET_CLASS_ATTRIBUTES(z);
	od_igbinary_serialize_object_name(igsd, class_name, name_len TSRMLS_CC);
	PHP_CLEANUP_CLASS_ATTRIBUTES();

	if (ce && ce != PHP_IC_ENTRY && zend_hash_exists(&ce->function_table, "__sleep", sizeof("__sleep"))) {
		/* function name string */
		INIT_PZVAL(&f);
		ZVAL_STRINGL(&f, "__sleep", sizeof("__sleep") - 1, 0);

		/* calling z->__sleep */
		r = call_user_function_ex(CG(function_table), &z, &f, &h, 0, 0, 1, NULL TSRMLS_CC);

		if (r == SUCCESS && !EG(exception)) {
			r = 0;

			if (h) {
				if (Z_TYPE_P(h) == IS_ARRAY) {
					r = od_igbinary_serialize_array_sleep(igsd, z, HASH_OF(h), ce, incomplete_class TSRMLS_CC);
				} else {
					od_error(E_ERROR, "__sleep should return an array only "
							"containing the names of instance-variables to "
							"serialize");

					/* empty array */
					od_igbinary_serialize8(igsd, od_igbinary_type_array8 TSRMLS_CC);
					r = od_igbinary_serialize8(igsd, 0 TSRMLS_CC);
				}
			}
		} else {
			r = 1;
		}

		/* cleanup */
		if (h) {
			zval_ptr_dtor(&h);
		}

		return r;
	} else {
		return od_igbinary_serialize_array(igsd, z, ce, true, incomplete_class, false TSRMLS_CC);
	}
}
/* }}} */
/* {{{ od_igbinary_serialize_zval */
/** Serialize zval. */
int od_igbinary_serialize_zval(od_igbinary_serialize_data *igsd, zval *z TSRMLS_DC) {
	if (Z_ISREF_P(z)) {
			// Depending on what behavior you want out of the extension you can enable this 
			// in /etc/php.d/odus.ini ( set odus.throw_exceptions=1 )
			if(ODUS_G(od_throw_exceptions)) {
					zend_throw_exception(odus_exception_ce, "Odus detected reference to VALUE which is not supported by odus serialization", -1 TSRMLS_CC);
					return 1; // return error
			}
			
			// This actually should be taken and put under another setting
			// but since od_reduce_fatals is used to return NULL on error
			// in the ..array_ref's paths continuing with that here.
			//
			// in /etc/php.d/odus.ini ( set odus.reduce_fatals=1 )
			if(ODUS_G(od_reduce_fatals)) { // default behavor of ODUS is to have reduce fatals off
					return 1; // return error
			}

			// DEFAULT behavior of ODUS 1.0.9 and before is to silently convert 
			// the Reference  into a copy of the value (this is what is deployed 
			// to citytc and the ville) so changing this behavior may affect those 
			// games, because of that I'm just leaving the default behavior as is.  
			// It is preferrable not to do though.
			//
			// Demo of the 1.0.9 behavior
			// $o = new stdClass;
			// $o->n = "only_one_string";
			// $o->r = &$o->n;
			//
			// $x = serialize($o);
			// echo "$x\n";
			// $y = od_serialize($o);
			// $z = new ODWrapper($y);
			// $j = serialize($z);
			// echo "$j\n";
			//
			// [schow@city-dev-12 tmp]$ php refref.php
			// O:8:"stdClass":2:{s:1:"n";s:15:"only_one_string";s:1:"r";R:2;}
			// O:8:"stdClass":2:{s:1:"n";s:15:"only_one_string";s:1:"r";s:15:"only_one_string";}
			// 
			// We can see that $z and $j has 2 independent copies of the "only_one_string".
			//
			// However, this is NOT desirable:
			//
			// BEST [Best option]: change ODUS to support references to VALUES (so $x == $j)
			// OK [Ok option]: don't support references but enable reduce_fatals=1 and throw_exceptions=1
			// UNDESIRABLE [Default option]: silently alter data during serialization (citytc,theville)
			//
			// od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_ref TSRMLS_CC);
	}

	switch (Z_TYPE_P(z)) {
		case IS_RESOURCE:
			return od_igbinary_serialize_null(igsd TSRMLS_CC);
		case IS_OBJECT:
			return od_igbinary_serialize_object(igsd, z TSRMLS_CC);
		case IS_ARRAY:
			return od_igbinary_serialize_array(igsd, z, NULL, false, false, false TSRMLS_CC);
		case IS_STRING:
			// We enable string compact for class name and property name only,
			// so call serialize_chararray for all literal strings.
			return od_igbinary_serialize_chararray_ex(igsd, Z_STRVAL_P(z), Z_STRLEN_P(z) TSRMLS_CC);
		case IS_LONG:
			return od_igbinary_serialize_long(igsd, Z_LVAL_P(z) TSRMLS_CC);
		case IS_NULL:
			return od_igbinary_serialize_null(igsd TSRMLS_CC);
		case IS_BOOL:
			return od_igbinary_serialize_bool(igsd, Z_LVAL_P(z) ? 1 : 0 TSRMLS_CC);
		case IS_DOUBLE:
			return od_igbinary_serialize_double(igsd, Z_DVAL_P(z) TSRMLS_CC);
		default:
			od_error(E_ERROR, "od_igbinary_serialize_zval: zval has unknown type %d", (int)Z_TYPE_P(z));
			/* not reached */
			return 1;
	}

	return 0;
}
/* }}} */
/* {{{ od_igbinary_unserialize_data_init */
/** Inits od_igbinary_unserialize_data_init. */
inline int od_igbinary_unserialize_data_init(od_igbinary_unserialize_data *igsd TSRMLS_DC) {
	//smart_str empty_str = { 0 };

	igsd->buffer = NULL;
	igsd->buffer_size = 0;
	igsd->buffer_offset = 0;

	igsd->strings = NULL;
	igsd->strings_count = 0;
	//igsd->strings_capacity = 4;

	igsd->compact_strings = true;

	igsd->compress_value_len = true;

	igsd->root_id = 0;

	//odus doesn't allow object references
	//igsd->references = NULL;
	//igsd->references_count = 0;
	//igsd->references_capacity = 4;

	//odus doesn't allow object references
	//igsd->references = (void **) emalloc(sizeof(void *) * igsd->references_capacity);
	//if (igsd->references == NULL) {
	//	return 1;
	//}

	// igsd->strings = (struct od_igbinary_unserialize_string_pair *) emalloc(sizeof(struct od_igbinary_unserialize_string_pair) * igsd->strings_capacity);
	// if (igsd->strings == NULL) {
	// 	//efree(igsd->references);
	// 	return 1;
	// }

	return 0;
}
/* }}} */

/* {{{ od_igbinary_unserialize_data_init */
/** Inits od_igbinary_unserialize_data_init. */
inline int od_igbinary_unserialize_data_clone(od_igbinary_unserialize_data *dest, od_igbinary_unserialize_data *src TSRMLS_DC) {
	if (!dest || !src) {
		debug("null pointer!\n");
		return 1;
	}

	dest->buffer = src->buffer;
	dest->buffer_size = src->buffer_size;
	dest->buffer_offset = src->buffer_offset;
	dest->strings = src->strings;
	dest->strings_count = src->strings_count;
	dest->original_buffer = src->original_buffer;
	dest->string_table_start = src->string_table_start;
	dest->strings_index_offset = src->strings_index_offset;
	dest->compact_strings = src->compact_strings;
	dest->compress_value_len = src->compress_value_len;
	dest->root_id = src->root_id;
	return 0;
}
/* }}} */

/* {{{ od_igbinary_unserialize_data_deinit */
/** Deinits od_igbinary_unserialize_data_init. */
inline void od_igbinary_unserialize_data_deinit(od_igbinary_unserialize_data *igsd TSRMLS_DC) {
	if (igsd->strings) {
		efree(igsd->strings);
	}

	return;
}
/* }}} */
/* {{{ od_igbinary_unserialize_header */
/** Unserialize header. Check for version. */
inline int od_igbinary_unserialize_header(od_igbinary_unserialize_data *igsd, uint32_t *version TSRMLS_DC) {
	if (igsd->buffer_offset + 4 >= igsd->buffer_size) {
		return 1;
	}

	*version = od_igbinary_unserialize32(igsd TSRMLS_CC);

	/* Support older version 1 and the current format 2 */
	if (*version == OD_IGBINARY_FORMAT_VERSION_01) {
		igsd->compact_strings = false;
		igsd->compress_value_len = false;
		return 0;
	} else if (*version == OD_IGBINARY_FORMAT_VERSION_02) {
		igsd->compact_strings = true;
		igsd->compress_value_len = true;
		return 0;
	} else {
		od_error(E_ERROR, "od_igbinary_unserialize_header: unsupported version: %u, should be %u", (unsigned int) *version,(unsigned int) OD_IGBINARY_FORMAT_VERSION);
		return 1;
	}
}
/* }}} */

/* {{{ od_igbinary_unserialize_init_string_table */
/** Init string table. */
inline int od_igbinary_unserialize_init_string_table(od_igbinary_unserialize_data *igsd TSRMLS_DC) {
	if (igsd->buffer_offset + 8 >= igsd->buffer_size) {
		return 1;
	}

	igsd->original_buffer = igsd->buffer;

	igsd->string_table_start = od_igbinary_unserialize32(igsd TSRMLS_CC);

	igsd->strings_index_offset = od_igbinary_unserialize32(igsd TSRMLS_CC);

	igsd->strings_count = od_igbinary_unserialize32_at(igsd, igsd->strings_index_offset - sizeof(uint32_t) TSRMLS_CC);

	igsd->strings = (struct od_igbinary_unserialize_string_pair *) emalloc(sizeof(struct od_igbinary_unserialize_string_pair) * igsd->strings_count);

	if (igsd->strings == NULL) {
		od_error(E_ERROR, "Fail to alloc memory for string table");
		return 1;
	}

	// For lazy loading.
	memset(igsd->strings, 0, sizeof(struct od_igbinary_unserialize_string_pair) * igsd->strings_count);

	return 0;
}
/* }}} */

/* {{{ od_igbinary_unserialize8_at */
/** Unserialize 8bit value at specified position. */
inline uint8_t od_igbinary_unserialize8_at(od_igbinary_unserialize_data *igsd, uint32_t pos TSRMLS_DC) {
	return igsd->buffer[pos];
}
/* }}} */

/* {{{ od_igbinary_unserialize8 */
/** Unserialize 8bit value. */
inline uint8_t od_igbinary_unserialize8(od_igbinary_unserialize_data *igsd TSRMLS_DC) {
	return igsd->buffer[igsd->buffer_offset++];
}
/* }}} */

/* {{{ od_igbinary_unserialize16 */
/** Unserialize 16bit value at specified position. */
inline uint16_t od_igbinary_unserialize16_at(od_igbinary_unserialize_data *igsd, uint32_t pos TSRMLS_DC) {
	uint16_t ret = 0;
	ret |= ((uint16_t) igsd->buffer[pos++] << 8);
	ret |= ((uint16_t) igsd->buffer[pos++] << 0);
	return ret;
}
/* }}} */

/* {{{ od_igbinary_unserialize16 */
/** Unserialize 16bit value. */
inline uint16_t od_igbinary_unserialize16(od_igbinary_unserialize_data *igsd TSRMLS_DC) {
	uint16_t ret = 0;
	ret |= ((uint16_t) igsd->buffer[igsd->buffer_offset++] << 8);
	ret |= ((uint16_t) igsd->buffer[igsd->buffer_offset++] << 0);
	return ret;
}
/* }}} */

/* {{{ od_igbinary_unserialize32 */
/** Unserialize 32bit value at specified position. */
inline uint32_t od_igbinary_unserialize32_at(od_igbinary_unserialize_data *igsd, uint32_t pos TSRMLS_DC) {
	uint32_t ret = 0;
	ret |= ((uint32_t) igsd->buffer[pos++] << 24);
	ret |= ((uint32_t) igsd->buffer[pos++] << 16);
	ret |= ((uint32_t) igsd->buffer[pos++] << 8);
	ret |= ((uint32_t) igsd->buffer[pos++] << 0);
	return ret;
}
/* }}} */

/* {{{ od_igbinary_unserialize32 */
/** Unserialize 32bit value. */
inline uint32_t od_igbinary_unserialize32(od_igbinary_unserialize_data *igsd TSRMLS_DC) {
	uint32_t ret = 0;
	ret |= ((uint32_t) igsd->buffer[igsd->buffer_offset++] << 24);
	ret |= ((uint32_t) igsd->buffer[igsd->buffer_offset++] << 16);
	ret |= ((uint32_t) igsd->buffer[igsd->buffer_offset++] << 8);
	ret |= ((uint32_t) igsd->buffer[igsd->buffer_offset++] << 0);
	return ret;
}
/* }}} */

/* {{{ od_igbinary_unserialize64 */
/** Unserialize 64bit value. */
inline uint64_t od_igbinary_unserialize64(od_igbinary_unserialize_data *igsd TSRMLS_DC) {
	uint64_t ret = 0;
	ret |= ((uint64_t) igsd->buffer[igsd->buffer_offset++] << 56);
	ret |= ((uint64_t) igsd->buffer[igsd->buffer_offset++] << 48);
	ret |= ((uint64_t) igsd->buffer[igsd->buffer_offset++] << 40);
	ret |= ((uint64_t) igsd->buffer[igsd->buffer_offset++] << 32);
	ret |= ((uint64_t) igsd->buffer[igsd->buffer_offset++] << 24);
	ret |= ((uint64_t) igsd->buffer[igsd->buffer_offset++] << 16);
	ret |= ((uint64_t) igsd->buffer[igsd->buffer_offset++] << 8);
	ret |= ((uint64_t) igsd->buffer[igsd->buffer_offset++] << 0);
	return ret;
}
/* }}} */
/* {{{ od_igbinary_unserialize_long */
/** Unserializes long */
inline int od_igbinary_unserialize_long(od_igbinary_unserialize_data *igsd, od_igbinary_type t, long *ret TSRMLS_DC) {


	uint32_t tmp32;
#if SIZEOF_LONG == 8
	uint64_t tmp64;
#endif

	if (t == od_igbinary_type_long8p || t == od_igbinary_type_long8n) {
		if (igsd->buffer_offset + 1 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_unserialize_long: end-of-data");
			return 1;
		}

		*ret = (long) (t == od_igbinary_type_long8n ? -1 : 1) * od_igbinary_unserialize8(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_long16p || t == od_igbinary_type_long16n) {
		if (igsd->buffer_offset + 2 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_unserialize_long: end-of-data");
			return 1;
		}

		*ret = (long) (t == od_igbinary_type_long16n ? -1 : 1) * od_igbinary_unserialize16(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_long32p || t == od_igbinary_type_long32n) {
		if (igsd->buffer_offset + 4 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_unserialize_long: end-of-data");
			return 1;
		}

		/* check for boundaries */
		tmp32 = od_igbinary_unserialize32(igsd TSRMLS_CC);
#if SIZEOF_LONG == 4
		if (tmp32 > 0x80000000 || (tmp32 == 0x80000000 && t == od_igbinary_type_long32p)) {
			od_error(E_ERROR, "od_igbinary_unserialize_long: 64bit long on 32bit platform?");
			tmp32 = 0; /* t == od_igbinary_type_long32p ? LONG_MAX : LONG_MIN; */
		}
#endif
		*ret = (long) (t == od_igbinary_type_long32n ? -1 : 1) * tmp32;
	} else if (t == od_igbinary_type_long64p || t == od_igbinary_type_long64n) {
#if SIZEOF_LONG == 8
		if (igsd->buffer_offset + 8 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_unserialize_long: end-of-data");
			return 1;
		}

		/* check for boundaries */
		tmp64 = od_igbinary_unserialize64(igsd TSRMLS_CC);
		if (tmp64 > 0x8000000000000000 || (tmp64 == 0x8000000000000000 && t == od_igbinary_type_long64p)) {
			od_error(E_ERROR, "od_igbinary_unserialize_long: too big 64bit long.");
			tmp64 = 0; /* t == od_igbinary_type_long64p ? LONG_MAX : LONG_MIN */;
		}

		*ret = (long) (t == od_igbinary_type_long64n ? -1 : 1) * tmp64;
#elif SIZEOF_LONG == 4
		/* can't put 64bit long into 32bit one, placeholder zero */
		*ret = 0;
		od_igbinary_unserialize64(igsd TSRMLS_CC);
		od_error(E_ERROR, "od_igbinary_unserialize_long: 64bit long on 32bit platform");
#else
#error "Strange sizeof(long)."
#endif
	} else {
		*ret = 0;
		od_error(E_ERROR, "od_igbinary_unserialize_long: unknown type '%02x', position %zu", t, igsd->buffer_offset);
		return 1;
	}

	return 0;
}
/* }}} */
/* {{{ od_igbinary_unserialize_double */
/** Unserializes double. */
inline int od_igbinary_unserialize_double(od_igbinary_unserialize_data *igsd, od_igbinary_type t, double *ret TSRMLS_DC) {


	union {
		double d;
		uint64_t u;
	} u;

	(void) t;

	if (igsd->buffer_offset + 8 > igsd->buffer_size) {
		od_error(E_ERROR, "od_igbinary_unserialize_double: end-of-data");
		return 1;
	}


	u.u = od_igbinary_unserialize64(igsd TSRMLS_CC);

	*ret = u.d;

	return 0;
}
/* }}} */

/* {{{ od_igbinary_unserialize_static_string */
/** Unserializes static string. */
inline int od_igbinary_unserialize_static_string(od_igbinary_unserialize_data *igsd, od_igbinary_type t, char **s, uint32_t *len TSRMLS_DC) {
	size_t i;

	if (t == od_igbinary_type_static_string_id8) {
		if (igsd->buffer_offset + 1 > igsd->buffer_size) {
			zend_error(E_WARNING, "od_igbinary_unserialize_static_string: end-of-data");
			return 1;
		}
		i = od_igbinary_unserialize8(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_static_string_id16) {
		if (igsd->buffer_offset + 2 > igsd->buffer_size) {
			zend_error(E_WARNING, "od_igbinary_unserialize_static_string: end-of-data");
			return 1;
		}
		i = od_igbinary_unserialize16(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_static_string_id32) {
		if (igsd->buffer_offset + 4 > igsd->buffer_size) {
			zend_error(E_WARNING, "od_igbinary_unserialize_static_string: end-of-data");
			return 1;
		}
		i = od_igbinary_unserialize32(igsd TSRMLS_CC);
	} else {
		zend_error(E_WARNING, "od_igbinary_unserialize_static_string: unknown type '%02x', position %zu", t, igsd->buffer_offset);
		return 1;
	}

	if (i >= od_static_strings_count) {
		zend_error(E_WARNING, "od_igbinary_unserialize_static_string: string index is out-of-bounds, i %d, strings_count %d", i, od_static_strings_count);
		return 1;
	}

	*s = od_static_strings[i];
	*len = strlen(od_static_strings[i]);

	return 0;
}
/* }}} */

/* {{{ od_igbinary_unserialize_string */
/** Unserializes string. Unserializes both actual string or by string id. */
inline static int od_igbinary_unserialize_string(od_igbinary_unserialize_data *igsd, od_igbinary_type t, char **s, uint32_t *len TSRMLS_DC) {
	size_t i;

	if (t == od_igbinary_type_string_id8 || t == od_igbinary_type_object_id8) {
		if (igsd->buffer_offset + 1 > igsd->buffer_size) {
			zend_error(E_WARNING, "od_igbinary_unserialize_string: end-of-data");
			return 1;
		}
		i = od_igbinary_unserialize8(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_string_id16 || t == od_igbinary_type_object_id16) {
		if (igsd->buffer_offset + 2 > igsd->buffer_size) {
			zend_error(E_WARNING, "od_igbinary_unserialize_string: end-of-data");
			return 1;
		}
		i = od_igbinary_unserialize16(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_string_id32 || t == od_igbinary_type_object_id32) {
		if (igsd->buffer_offset + 4 > igsd->buffer_size) {
			zend_error(E_WARNING, "od_igbinary_unserialize_string: end-of-data");
			return 1;
		}
		i = od_igbinary_unserialize32(igsd TSRMLS_CC);
	} else {
		zend_error(E_WARNING, "od_igbinary_unserialize_string: unknown type '%02x', position %zu", t, igsd->buffer_offset);
		return 1;
	}

	if (i >= igsd->strings_count) {
		zend_error(E_WARNING, "od_igbinary_unserialize_string: string index is out-of-bounds, i %d, strings_count %d", i, igsd->strings_count);
		return 1;
	}

	// Lazy loading the string.
	if (!igsd->strings[i].data) {
		// unserializing functions are tied to igsd->buffer. Tune it here to make them happy.
		uint8_t *buffer_backup = igsd->buffer;
		igsd->buffer = igsd->original_buffer;

		uint32_t index_offset = igsd->strings_index_offset + i * sizeof(uint32_t);
		uint32_t string_offset = od_igbinary_unserialize32_at(igsd, index_offset TSRMLS_CC) + igsd->string_table_start;
		od_igbinary_unserialize_string_from_table(igsd, &igsd->strings[i].data, &igsd->strings[i].len, string_offset TSRMLS_CC);

		// Restore current buffer.
		igsd->buffer = buffer_backup;

		if (!igsd->strings[i].data) {
			return 1;
		}
	}

	*s = igsd->strings[i].data;
	*len = igsd->strings[i].len;

	return 0;
}
/* }}} */

/* {{{ od_igbinary_unserialize_chararray */
/** Unserializes chararray of string. */
inline int od_igbinary_unserialize_chararray(od_igbinary_unserialize_data *igsd, od_igbinary_type t, char **s, uint32_t *len TSRMLS_DC) {
	uint32_t l;

	//XXX
	// in odus string ends with '\0'
	if (t == od_igbinary_type_string8 || t == od_igbinary_type_object8) {
		if (igsd->buffer_offset + 2 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_unserialize_chararray: end-of-data");
			return 1;
		}
		l = od_igbinary_unserialize8(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_string16 || t == od_igbinary_type_object16) {
		if (igsd->buffer_offset + 3 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_unserialize_chararray: end-of-data");
			return 1;
		}
		l = od_igbinary_unserialize16(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_string32 || t == od_igbinary_type_object32) {
		if (igsd->buffer_offset + 5 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_unserialize_chararray: end-of-data");
			return 1;
		}
		l = od_igbinary_unserialize32(igsd TSRMLS_CC);
	} else {
		od_error(E_ERROR, "od_igbinary_unserialize_chararray: unknown type '%02x', position %zu", t, igsd->buffer_offset);
		return 1;
	}

	*s = (char *) (igsd->buffer + igsd->buffer_offset);
	*len = l;

	//XXX
	// in odus string ends with '\0'
	igsd->buffer_offset += l+1;

	return 0;
}
/* }}} */

/* {{{ od_igbinary_unserialize_class_name */
/** Unserializes class name. */
inline int od_igbinary_unserialize_class_name(od_igbinary_unserialize_data *igsd, od_igbinary_type t, char **name, uint32_t *name_len TSRMLS_DC) {
	if (t == od_igbinary_type_object8 || t == od_igbinary_type_object16 || t == od_igbinary_type_object32) {
		return od_igbinary_unserialize_chararray(igsd, t, name, name_len TSRMLS_CC);
	} else if (t == od_igbinary_type_object_id8 || t == od_igbinary_type_object_id16 || t == od_igbinary_type_object_id32) {
		return od_igbinary_unserialize_string(igsd, t, name, name_len TSRMLS_CC);
	}
	else {
		od_error(E_ERROR, "od_igbinary_unserialize_class_name: unknown object type '%02x', position %zu", t, igsd->buffer_offset);
		return 1;
	}
}
/* }}} */


/* {{{ od_igbinary_unserialize_string_from_table */
/** Unserializes chararray of string from the string table. */
inline int od_igbinary_unserialize_string_from_table(od_igbinary_unserialize_data *igsd, char **s, uint32_t *len, uint32_t pos TSRMLS_DC) {
	uint32_t l;
	uint32_t offset = pos;

	od_igbinary_type t = (od_igbinary_type) od_igbinary_unserialize8_at(igsd, offset TSRMLS_CC);

	offset += 1;
	if (t == od_igbinary_type_string8 || t == od_igbinary_type_object8) {
		// Ignore the check, because buffer_size may be small for an inside object. Just make sure
		// the buffer start and offset is correct on calling.

		// if (offset + 2 > igsd->buffer_size) {
		// 	od_error(E_ERROR, "od_igbinary_unserialize_string_from_table: end-of-data");
		// 	return 1;
		// }
		l = od_igbinary_unserialize8_at(igsd, offset TSRMLS_CC);
		offset += 1;
	} else if (t == od_igbinary_type_string16 || t == od_igbinary_type_object16) {
		// if (offset + 3 > igsd->buffer_size) {
		// 	od_error(E_ERROR, "od_igbinary_unserialize_string_from_table: end-of-data");
		// 	return 1;
		// }
		l = od_igbinary_unserialize16_at(igsd, offset TSRMLS_CC);
		offset += 2;
	} else if (t == od_igbinary_type_string32 || t == od_igbinary_type_object32) {
		// if (offset + 5 > igsd->buffer_size) {
		// 	od_error(E_ERROR, "od_igbinary_unserialize_string_from_table: end-of-data");
		// 	return 1;
		// }
		l = od_igbinary_unserialize32_at(igsd, offset TSRMLS_CC);
		offset += 4;
	} else {
		od_error(E_ERROR, "od_igbinary_unserialize_string_from_table: unknown type '%02x', position %zu", t, igsd->buffer_offset);
		return 1;
	}

	*s = (char *) (igsd->buffer + offset);
	*len = l;

	return 0;
}
/* }}} */

inline od_igbinary_type od_igbinary_get_type(od_igbinary_unserialize_data *igsd) {
	if (igsd->buffer_offset + 1 > igsd->buffer_size) {
		debug("wrong in %s:%d",__FUNCTION__,__LINE__);
		od_error(E_ERROR, "od_igbinary_get_type: end-of-data");
		return od_igbinary_type_null;
	}

	return (od_igbinary_type) od_igbinary_unserialize8(igsd TSRMLS_CC);
}

inline od_igbinary_type od_igbinary_get_type_at(od_igbinary_unserialize_data *igsd, uint32_t pos) {
	if (pos + 1 > igsd->buffer_size) {
		debug("wrong in %s:%d",__FUNCTION__,__LINE__);
		od_error(E_ERROR, "od_igbinary_get_type_at: end-of-data");
		return od_igbinary_type_null;
	}

	return (od_igbinary_type) od_igbinary_unserialize8_at(igsd, pos TSRMLS_CC);
}

inline uint32_t od_igbinary_get_member_num(od_igbinary_unserialize_data *igsd, od_igbinary_type t) {

	uint32_t n;

	if (t == od_igbinary_type_array8) {
		if (igsd->buffer_offset + 1 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_get_member_num: end-of-data");
			return -1;
		}
		n = od_igbinary_unserialize8(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_array16) {
		if (igsd->buffer_offset + 2 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_get_member_num: end-of-data");
			return -1;
		}
		n = od_igbinary_unserialize16(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_array32) {
		if (igsd->buffer_offset + 4 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_get_member_num: end-of-data");
			return -1;
		}
		n = od_igbinary_unserialize32(igsd TSRMLS_CC);
	} else {
		od_error(E_ERROR, "od_igbinary_get_member_num: unknown type '%02x', position %zu", t, igsd->buffer_offset);
		return -1;
	}

	// n cannot be larger than the number of minimum "objects" in the array
	if (n > igsd->buffer_size - igsd->buffer_offset) {
		od_error(E_ERROR, "%s: data size %zu smaller that requested array length %zu.", "od_igbinary_get_member_num", igsd->buffer_size - igsd->buffer_offset, n);
		return -1;
	}

	return n;
}

inline uint32_t od_igbinary_get_value_len(od_igbinary_unserialize_data *igsd) {

	if (igsd->compress_value_len) {
		uint8_t encode_len_probe = 0;

		if (igsd->buffer_offset + 1 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_get_value_len: end-of-data");
			return -1;
		}
		encode_len_probe = od_igbinary_unserialize8_at(igsd, igsd->buffer_offset);

		if ((encode_len_probe & 0xc0) == 0x40) {
			igsd->buffer_offset += 1;

			return encode_len_probe & 0x3f;
		} else if ((encode_len_probe & 0xc0) == 0x80) {
			if (igsd->buffer_offset + 2 > igsd->buffer_size) {
				od_error(E_ERROR, "od_igbinary_get_value_len: end-of-data");
				return -1;
			}

			uint16_t value_len = od_igbinary_unserialize16(igsd);
			return value_len & 0x3fff;
		} else {
			if (igsd->buffer_offset + 4 > igsd->buffer_size) {
				od_error(E_ERROR, "od_igbinary_get_value_len: end-of-data");
				return -1;
			}

			return (uint32_t)od_igbinary_unserialize32(igsd);
		}
	} else {
		if (igsd->buffer_offset + OD_IGBINARY_VALUE_LEN_SIZE > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_get_value_len: end-of-data");
			return -1;
		}

		return (uint32_t)od_igbinary_unserialize32(igsd);
	}
}

inline int od_igbinary_skip_value_len(od_igbinary_unserialize_data *igsd) {
	if (igsd->compress_value_len) {
		uint8_t encode_len_probe = 0;

		if (igsd->buffer_offset + 1 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_skip_value_len: end-of-data");
			return -1;
		}
		encode_len_probe = od_igbinary_unserialize8_at(igsd, igsd->buffer_offset);

		if ((encode_len_probe & 0xc0) == 0x40) {
			igsd->buffer_offset += 1;
			return 0;
		} else if ((encode_len_probe & 0xc0) == 0x80) {
			if (igsd->buffer_offset + 2 > igsd->buffer_size) {
				od_error(E_ERROR, "od_igbinary_skip_value_len: end-of-data");
				return -1;
			}

			igsd->buffer_offset += 2;
			return 0;
		} else {
			if (igsd->buffer_offset + 4 > igsd->buffer_size) {
				od_error(E_ERROR, "od_igbinary_skip_value_len: end-of-data");
				return -1;
			}

			igsd->buffer_offset += 4;
			return 0;
		}
	} else {

		if (igsd->buffer_offset + OD_IGBINARY_VALUE_LEN_SIZE > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_skip_value_len: end-of-data");
			return 1;
		}

		igsd->buffer_offset += OD_IGBINARY_VALUE_LEN_SIZE;

		return 0;
	}
}

inline int od_igbinary_unserialize_skip_key(od_igbinary_unserialize_data *igsd) {
	char* key;
	uint32_t key_len;
	long key_index;

	return od_igbinary_unserialize_get_key(igsd,&key,&key_len,&key_index);
}

inline int od_igbinary_unserialize_get_key(od_igbinary_unserialize_data *igsd, char** key_p, uint32_t* key_len_p, long* key_index_p) {


	*key_p = NULL;

	od_igbinary_type key_type = od_igbinary_get_type(igsd TSRMLS_CC);

	switch (key_type) {
		case od_igbinary_type_string_id8:
		case od_igbinary_type_string_id16:
		case od_igbinary_type_string_id32:
			if (od_igbinary_unserialize_string(igsd, key_type, key_p, key_len_p TSRMLS_CC)) {
				return 1;
			}
			break;

		case od_igbinary_type_string8:
		case od_igbinary_type_string16:
		case od_igbinary_type_string32:
			if (od_igbinary_unserialize_chararray(igsd, key_type, key_p, key_len_p TSRMLS_CC)) {
				return 1;
			}
			break;
		case od_igbinary_type_static_string_id8:
		case od_igbinary_type_static_string_id16:
		case od_igbinary_type_static_string_id32:
			if (od_igbinary_unserialize_static_string(igsd, key_type, key_p, key_len_p TSRMLS_CC)) {
				return 1;
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
			if (od_igbinary_unserialize_long(igsd, key_type, key_index_p TSRMLS_CC)) {
				return 1;
			}
			break;
		case od_igbinary_type_string_empty:
		case od_igbinary_type_null:
			*key_p = EMPTY_STRING;
			*key_len_p = 0;
			*key_index_p = 0;
			break;
		default:
			od_error(E_ERROR, "od_igbinary_unserialize_get_key: unknown key type '%02x', position %zu", key_type, igsd->buffer_offset);
			return 1;
	}

	return 0;
}

/* {{{ od_igbinary_unserialize_array */
/** Unserializes array. */
inline static int od_igbinary_unserialize_array(od_igbinary_unserialize_data *igsd, od_igbinary_type t, zval **z, int object TSRMLS_DC) {


	uint32_t i;

	zval *v = NULL;
	/*	zval *old_v; */

	char *key;
	uint32_t key_len = 0;
	long key_index = 0;

	od_igbinary_type key_type;

	HashTable *h;

	uint32_t n = od_igbinary_get_member_num(igsd,t);

	if(n<0) return 1;

	if (!object) {
		Z_TYPE_PP(z) = IS_ARRAY;
		ALLOC_HASHTABLE(Z_ARRVAL_PP(z));
		zend_hash_init(Z_ARRVAL_PP(z), n + 1, NULL, ZVAL_PTR_DTOR, 0);
	}

	if(od_igbinary_skip_value_len(igsd)) {
		zval_dtor(*z);
		ZVAL_NULL(*z);
		return 1;
	}

	/* empty array */
	if (n == 0) {
		return 0;
	}

	h = HASH_OF(*z);

	for (i = 0; i < n; i++) {
		if(od_igbinary_unserialize_get_key(igsd,&key,&key_len,&key_index)){
			zval_dtor(*z);
			ZVAL_NULL(*z);
			return 1;
		}

		ALLOC_INIT_ZVAL(v);
		if (od_igbinary_unserialize_zval(igsd, &v TSRMLS_CC)) {
			zval_dtor(*z);
			ZVAL_NULL(*z);
			zval_ptr_dtor(&v);
			return 1;
		}

		if (key) {
			zend_symtable_update(h, key, key_len + 1, &v, sizeof(v), NULL);
		} else {
			zend_hash_index_update(h, key_index, &v, sizeof(v), NULL);
		}
	}

	return 0;
}
/* }}} */
/* {{{ od_igbinary_unserialize_object_ser */
/** Unserializes object's property array of objects implementing Serializable -interface. */
inline static int od_igbinary_unserialize_object_ser(od_igbinary_unserialize_data *igsd, od_igbinary_type t, zval **z, zend_class_entry *ce TSRMLS_DC) {
	uint32_t n;

	if (ce->unserialize == NULL) {
		od_error(E_ERROR, "Class %s has no unserializer", ce->name);
		return 1;
	}

	if (t == od_igbinary_type_object_ser8) {
		if (igsd->buffer_offset + 1 > igsd->buffer_size) {
			debug("wrong in %s:%d",__FUNCTION__,__LINE__);od_error(E_ERROR, "od_igbinary_unserialize_object_ser: end-of-data");
			return 1;
		}
		n = od_igbinary_unserialize8(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_object_ser16) {
		if (igsd->buffer_offset + 2 > igsd->buffer_size) {
			debug("wrong in %s:%d",__FUNCTION__,__LINE__);od_error(E_ERROR, "od_igbinary_unserialize_object_ser: end-of-data");
			return 1;
		}
		n = od_igbinary_unserialize16(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_object_ser32) {
		if (igsd->buffer_offset + 4 > igsd->buffer_size) {
			debug("wrong in %s:%d",__FUNCTION__,__LINE__);od_error(E_ERROR, "od_igbinary_unserialize_object_ser: end-of-data");
			return 1;
		}
		n = od_igbinary_unserialize32(igsd TSRMLS_CC);
	} else {
		od_error(E_ERROR, "od_igbinary_unserialize_object_ser: unknown type '%02x', position %zu", t, igsd->buffer_offset);
		return 1;
	}

	if (igsd->buffer_offset + n > igsd->buffer_size) {
		debug("wrong in %s:%d",__FUNCTION__,__LINE__);od_error(E_ERROR, "od_igbinary_unserialize_object_ser: end-of-data");
		return 1;
	}

	if (ce->unserialize(z, ce, (const unsigned char*)(igsd->buffer + igsd->buffer_offset), n, NULL TSRMLS_CC) != SUCCESS) {
		return 1;
	} else if (EG(exception)) {
		return 1;
	}

	igsd->buffer_offset += n;

	return 0;
}
/* }}} */
/* {{{ od_igbinary_unserialize_object */
/** Unserialize object.
 * @see ext/standard/var_unserializer.c
 */
inline static int od_igbinary_unserialize_object(od_igbinary_unserialize_data *igsd, od_igbinary_type t, zval **z TSRMLS_DC) {


	zend_class_entry *ce;
	zend_class_entry **pce;

	zval *h = NULL;
	zval f;

	char *name = NULL;
	uint32_t name_len = 0;

	int r;

	bool incomplete_class = false;

	zval *user_func;
	zval *retval_ptr;
	zval **args[1];
	zval *arg_func_name;

	if (od_igbinary_unserialize_class_name(igsd, t, &name, &name_len TSRMLS_CC)) {
		return 1;
	}

	do {
		/* Try to find class directly */
		if (zend_lookup_class(name, name_len, &pce TSRMLS_CC) == SUCCESS) {
			ce = *pce;
			break;
		}

		//XXX -- xma
		//in zend_lookup_class will do the autoload things

		/* Check for unserialize callback */
		if ((PG(unserialize_callback_func) == NULL) || (PG(unserialize_callback_func)[0] == '\0')) {
			incomplete_class = 1;
			ce = PHP_IC_ENTRY;
			break;
		}

		/* Call unserialize callback */
		MAKE_STD_ZVAL(user_func);
		ZVAL_STRING(user_func, PG(unserialize_callback_func), 1);
		args[0] = &arg_func_name;
		MAKE_STD_ZVAL(arg_func_name);
		ZVAL_STRING(arg_func_name, name, 1);
		if (call_user_function_ex(CG(function_table), NULL, user_func, &retval_ptr, 1, args, 0, NULL TSRMLS_CC) != SUCCESS) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "defined (%s) but not found", name);
			incomplete_class = 1;
			ce = PHP_IC_ENTRY;
			zval_ptr_dtor(&user_func);
			zval_ptr_dtor(&arg_func_name);
			break;
		}
		if (retval_ptr) {
			zval_ptr_dtor(&retval_ptr);
		}

		/* The callback function may have defined the class */
		if (zend_lookup_class(name, name_len, &pce TSRMLS_CC) == SUCCESS) {
			ce = *pce;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Function %s() hasn't defined the class it was called for", name);
			incomplete_class = true;
			ce = PHP_IC_ENTRY;
		}

		zval_ptr_dtor(&user_func);
		zval_ptr_dtor(&arg_func_name);
	} while (0);

	/* previous user function call may have raised an exception */
	if (EG(exception)) {
		return 1;
	}

	object_init_ex(*z, ce);

	//odus doesn't allow object references
	/* reference */
	//if (igsd->references_count + 1 >= igsd->references_capacity) {
	//	while (igsd->references_count + 1 >= igsd->references_capacity) {
	//		igsd->references_capacity *= 2;
	//	}

	//	igsd->references = (void **) erealloc(igsd->references, sizeof(void *) * igsd->references_capacity);
	//	if (igsd->references == NULL)
	//		return 1;
	//}

	//igsd->references[igsd->references_count++] = (void *) *z;

	/* store incomplete class name */
	if (incomplete_class) {
		//XXX
		// odus doesn't allow incomplete class
		od_error(E_ERROR,"could not find define for class '%s'",name);
		php_store_class_name(*z, name, name_len);
	}

	t = (od_igbinary_type) od_igbinary_unserialize8(igsd TSRMLS_CC);

	switch (t) {
		case od_igbinary_type_array8:
		case od_igbinary_type_array16:
		case od_igbinary_type_array32:
			r = od_igbinary_unserialize_array(igsd, t, z, 1 TSRMLS_CC);
			break;
			/*
			 * odus doesn't allow class has its serialization method
		case od_igbinary_type_object_ser8:
		case od_igbinary_type_object_ser16:
		case od_igbinary_type_object_ser32:
			r = od_igbinary_unserialize_object_ser(igsd, t, z, ce TSRMLS_CC);
			break;
			*/
		default:
			od_error(E_ERROR, "od_igbinary_unserialize_object: unknown object inner type '%02x', position %zu", t, igsd->buffer_offset);
			return 1;
	}

	if (r) {
		return r;
	}

	if (Z_OBJCE_PP(z) != PHP_IC_ENTRY && zend_hash_exists(&Z_OBJCE_PP(z)->function_table, "__wakeup", sizeof("__wakeup"))) {
		INIT_PZVAL(&f);
		ZVAL_STRINGL(&f, "__wakeup", sizeof("__wakeup") - 1, 0);
		call_user_function_ex(CG(function_table), z, &f, &h, 0, 0, 1, NULL TSRMLS_CC);

		if (h) {
			zval_ptr_dtor(&h);
		}

		if (EG(exception)) {
			r = 1;
		}
	}

	return r;
}
/* }}} */

/* {{{ od_igbinary_unserialize_zval */
/** Unserialize zval. */
int od_igbinary_unserialize_zval(od_igbinary_unserialize_data *igsd, zval **z TSRMLS_DC) {
	od_igbinary_type t;


	long tmp_long;
	double tmp_double;
	char *tmp_chararray;
	uint32_t tmp_uint32_t;

	if (igsd->buffer_offset + 1 > igsd->buffer_size) {
		debug("wrong in %s:%d",__FUNCTION__,__LINE__);
		od_error(E_ERROR, "od_igbinary_unserialize_zval: end-of-data");
		return 1;
	}

	t = (od_igbinary_type) od_igbinary_unserialize8(igsd TSRMLS_CC);

	switch (t) {
		case od_igbinary_type_object8:
		case od_igbinary_type_object16:
		case od_igbinary_type_object32:
		case od_igbinary_type_object_id8:
		case od_igbinary_type_object_id16:
		case od_igbinary_type_object_id32:
			if (od_igbinary_unserialize_object(igsd, t, z TSRMLS_CC)) {
				return 1;
			}
			break;
		case od_igbinary_type_array8:
		case od_igbinary_type_array16:
		case od_igbinary_type_array32:
			if (od_igbinary_unserialize_array(igsd, t, z, 0 TSRMLS_CC)) {
				return 1;
			}
			break;
		case od_igbinary_type_string8:
		case od_igbinary_type_string16:
		case od_igbinary_type_string32:
			if (od_igbinary_unserialize_chararray(igsd, t, &tmp_chararray, &tmp_uint32_t TSRMLS_CC)) {
				return 1;
			}
			ZVAL_STRINGL(*z, tmp_chararray, tmp_uint32_t, 1);
			break;
		case od_igbinary_type_string_empty:
			ZVAL_EMPTY_STRING(*z);
			break;
		case od_igbinary_type_string_id8:
		case od_igbinary_type_string_id16:
		case od_igbinary_type_string_id32:
			if (od_igbinary_unserialize_string(igsd, t, &tmp_chararray, &tmp_uint32_t TSRMLS_CC)) {
				return 1;
			}
			ZVAL_STRINGL(*z, tmp_chararray, tmp_uint32_t, 1);
			break;
		case od_igbinary_type_static_string_id8:
		case od_igbinary_type_static_string_id16:
		case od_igbinary_type_static_string_id32:
			if (od_igbinary_unserialize_static_string(igsd, t, &tmp_chararray, &tmp_uint32_t TSRMLS_CC)) {
				return 1;
			}
			ZVAL_STRINGL(*z, tmp_chararray, tmp_uint32_t, 1);
			break;
		case od_igbinary_type_long8p:
		case od_igbinary_type_long8n:
		case od_igbinary_type_long16p:
		case od_igbinary_type_long16n:
		case od_igbinary_type_long32p:
		case od_igbinary_type_long32n:
		case od_igbinary_type_long64p:
		case od_igbinary_type_long64n:
			if (od_igbinary_unserialize_long(igsd, t, &tmp_long TSRMLS_CC)) {
				return 1;
			}
			ZVAL_LONG(*z, tmp_long);
			break;
		case od_igbinary_type_null:
			ZVAL_NULL(*z);
			break;
		case od_igbinary_type_bool_false:
			ZVAL_BOOL(*z, 0);
			break;
		case od_igbinary_type_bool_true:
			ZVAL_BOOL(*z, 1);
			break;
		case od_igbinary_type_double:
			if (od_igbinary_unserialize_double(igsd, t, &tmp_double TSRMLS_CC)) {
				return 1;
			}
			ZVAL_DOUBLE(*z, tmp_double);
			break;
		default:
			od_error(E_ERROR, "od_igbinary_unserialize_zval: unknown type '%02x', position %zu", t, igsd->buffer_offset);
			return 1;
	}

	return 0;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 2
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
