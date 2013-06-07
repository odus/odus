#ifndef _OD_IGBINARY_H_
#define _OD_IGBINARY_H_

#include "od_def.h"

#include "hash.h"

/** Backport macros from php 5.3 */
#ifndef Z_ISREF_P
#define Z_ISREF_P(pz)                  PZVAL_IS_REF(pz)
#endif

#ifndef Z_ISREF_PP
#define Z_ISREF_PP(ppz)                Z_ISREF_P(*(ppz))
#endif

#ifndef Z_SET_ISREF_TO_P
#define Z_SET_ISREF_TO_P(pz, isref)    (Z_ISREF_P(pz) = (isref))
#endif

#ifndef Z_SET_ISREF_TO_PP
#define Z_SET_ISREF_TO_PP(ppz, isref)  Z_SET_ISREF_TO_P(*(ppz), isref)
#endif

#ifndef Z_ADDREF_P
#define Z_ADDREF_P(pz)                 ZVAL_ADDREF(pz)
#endif

#ifndef Z_ADDREF_PP
#define Z_ADDREF_PP(ppz)               Z_ADDREF_P(*(ppz))
#endif

//XXX
//must be 4
#define OD_IGBINARY_VALUE_LEN_SIZE 4

#define OD_IGBINARY_STRING_TABLE_START_OFFSET 4
#define OD_IGBINARY_STRING_TABLE_INDEX_OFFSET 8
#define OD_IGBINARY_STRING_TABLE_INFO_LEN 8

#define OD_RESERVED_BUFFER_LEN 1024

#define OD_uint32_t uint32_t

/* {{{ Types */
typedef enum od_igbinary_type_e {
	/* 00 */ od_igbinary_type_null,			/**< Null. */

	/* 01 */ od_igbinary_type_ref8,			/**< Array reference. */
	/* 02 */ od_igbinary_type_ref16,			/**< Array reference. */
	/* 03 */ od_igbinary_type_ref32,			/**< Array reference. */

	/* 04 */ od_igbinary_type_bool_false,		/**< Boolean true. */
	/* 05 */ od_igbinary_type_bool_true,		/**< Boolean false. */

	/* 06 */ od_igbinary_type_long8p,			/**< Long 8bit positive. */
	/* 07 */ od_igbinary_type_long8n,			/**< Long 8bit negative. */
	/* 08 */ od_igbinary_type_long16p,			/**< Long 16bit positive. */
	/* 09 */ od_igbinary_type_long16n,			/**< Long 16bit negative. */
	/* 0a */ od_igbinary_type_long32p,			/**< Long 32bit positive. */
	/* 0b */ od_igbinary_type_long32n,			/**< Long 32bit negative. */

	/* 0c */ od_igbinary_type_double,			/**< Double. */

	/* 0d */ od_igbinary_type_string_empty,	/**< Empty string. */

	/* 0e */ od_igbinary_type_string_id8,		/**< String id. */
	/* 0f */ od_igbinary_type_string_id16,		/**< String id. */
	/* 10 */ od_igbinary_type_string_id32,		/**< String id. */

	/* 11 */ od_igbinary_type_string8,			/**< String. */
	/* 12 */ od_igbinary_type_string16,		/**< String. */
	/* 13 */ od_igbinary_type_string32,		/**< String. */

	/* 14 */ od_igbinary_type_array8,			/**< Array. */
	/* 15 */ od_igbinary_type_array16,			/**< Array. */
	/* 16 */ od_igbinary_type_array32,			/**< Array. */

	/* 17 */ od_igbinary_type_object8,			/**< Object. */
	/* 18 */ od_igbinary_type_object16,		/**< Object. */
	/* 19 */ od_igbinary_type_object32,		/**< Object. */

	/* 1a */ od_igbinary_type_object_id8,		/**< Object string id. */
	/* 1b */ od_igbinary_type_object_id16,		/**< Object string id. */
	/* 1c */ od_igbinary_type_object_id32,		/**< Object string id. */

	/* 1d */ od_igbinary_type_object_ser8,		/**< Object serialized data. */
	/* 1e */ od_igbinary_type_object_ser16,	/**< Object serialized data. */
	/* 1f */ od_igbinary_type_object_ser32,	/**< Object serialized data. */

	/* 20 */ od_igbinary_type_long64p,			/**< Long 64bit positive. */
	/* 21 */ od_igbinary_type_long64n,			/**< Long 64bit negative. */

	/* 22 */ od_igbinary_type_objref8,			/**< Object reference. */
	/* 23 */ od_igbinary_type_objref16,		/**< Object reference. */
	/* 24 */ od_igbinary_type_objref32,		/**< Object reference. */

	/* 25 */ od_igbinary_type_ref,				/**< Simple reference */

	/* 26 */ od_igbinary_type_static_string_id8,		/**< Static string id. */
	/* 27 */ od_igbinary_type_static_string_id16,		/**< Static string id. */
	/* 28 */ od_igbinary_type_static_string_id32,		/**< Static string id. */

	/* 29 */ od_igbinary_type_object_static_string_id8,		/**< Static string id. */
	/* 2a */ od_igbinary_type_object_static_string_id16,		/**< Static string id. */
	/* 2b */ od_igbinary_type_object_static_string_id32,		/**< Static string id. */
} od_igbinary_type;

/** Serializer data.
 * @author Oleg Grenrus <oleg.grenrus@dynamoid.com>
 */
typedef struct od_igbinary_serialize_data_s {
	uint8_t *buffer;			/**< Buffer. */
	uint32_t buffer_size;			/**< Buffer size. */
	uint32_t buffer_capacity;		/**< Buffer capacity. */
	bool scalar;				/**< Serializing scalar. */
	bool compact_strings;		/**< Compact strings(currently for class name and property names) into a string table. */
	struct hash_si strings;		/**< Hash of already serialized strings. */
	struct hash_si objects;		/**< Hash of already serialized objects. */

	int strings_count;			/**< Serialized string count, used for back referencing */
	bool string_table_update;	/**< Whether the string table has been updated, used to update string table when serializing an ODWrapper object. */

	bool compress_value_len;	/**< Whether compress the value len (into 1, 2 or 4 bytes) */

	uint64_t root_id;
}od_igbinary_serialize_data;

/** String/len pair for the od_igbinary_unserializer_data.
 * @author Oleg Grenrus <oleg.grenrus@dynamoid.com>
 * @see od_igbinary_unserialize_data.
 */
struct od_igbinary_unserialize_string_pair {
	char *data;		/**< Data. */
	uint32_t len;		/**< Data length. */
};

/** Unserializer data.
 * @author Oleg Grenrus <oleg.grenrus@dynamoid.com>
 */
typedef struct od_igbinary_unserialize_data_s {
	uint8_t *buffer;				/**< Address of current object in blob, will change in recursive unserializing. */
	uint32_t buffer_size;				/**< Buffer size. */
	uint32_t buffer_offset;			/**< Current read offset. */

	// Properties for string table. They will stay unchanged in recursive unserializing.
	struct od_igbinary_unserialize_string_pair *strings; /**< Unserialized strings. */
	size_t strings_count;			/**< Unserialized string count. */
	//size_t strings_capacity;		/**< Unserialized string array capacity. */
	uint8_t *original_buffer;		/**< Original address of entire blob. */
	uint32_t string_table_start;		/**< Offset for the start pos of string table. */
	uint32_t strings_index_offset;		/**< Offset of the string indexes, related to original_buffer. */
	//smart_str string0_buf;			/**< Temporary buffer for strings */
	bool compact_strings;		/**< Check for duplicate strings. */

	bool compress_value_len;	/**< Whether compress the value len (into 1, 2 or 4 bytes) */

	uint64_t root_id;
} od_igbinary_unserialize_data;
/* }}} */

inline int od_igbinary_init(TSRMLS_D);
inline int od_igbinary_shutdown(TSRMLS_D);

inline int od_igbinary_unserialize_chararray(od_igbinary_unserialize_data *igsd, od_igbinary_type t, char **s, uint32_t *len TSRMLS_DC);
inline int od_igbinary_unserialize_long(od_igbinary_unserialize_data *igsd, od_igbinary_type t, long *ret TSRMLS_DC);
inline int od_igbinary_unserialize_double(od_igbinary_unserialize_data *igsd, od_igbinary_type t, double *ret TSRMLS_DC);

inline int od_igbinary_unserialize_static_string(od_igbinary_unserialize_data *igsd, od_igbinary_type t, char **s, uint32_t *len TSRMLS_DC);

inline int od_igbinary_unserialize_class_name(od_igbinary_unserialize_data *igsd, od_igbinary_type t, char **name, uint32_t *name_len TSRMLS_DC);

inline uint8_t od_igbinary_unserialize8(od_igbinary_unserialize_data *igsd TSRMLS_DC);
inline uint16_t od_igbinary_unserialize16(od_igbinary_unserialize_data *igsd TSRMLS_DC);
inline uint32_t od_igbinary_unserialize32(od_igbinary_unserialize_data *igsd TSRMLS_DC);
inline uint64_t od_igbinary_unserialize64(od_igbinary_unserialize_data *igsd TSRMLS_DC);

inline uint8_t od_igbinary_unserialize8_at(od_igbinary_unserialize_data *igsd, uint32_t pos TSRMLS_DC);
inline uint16_t od_igbinary_unserialize16_at(od_igbinary_unserialize_data *igsd, uint32_t pos TSRMLS_DC);
inline uint32_t od_igbinary_unserialize32_at(od_igbinary_unserialize_data *igsd, uint32_t pos TSRMLS_DC);
inline uint64_t od_igbinary_unserialize64_at(od_igbinary_unserialize_data *igsd, uint32_t pos TSRMLS_DC);

inline uint32_t od_igbinary_get_value_len(od_igbinary_unserialize_data *igsd);

inline int od_igbinary_skip_value_len(od_igbinary_unserialize_data *igsd);

inline uint32_t od_igbinary_get_member_num(od_igbinary_unserialize_data *igsd, od_igbinary_type t);

inline int od_igbinary_serialize8(od_igbinary_serialize_data *igsd, uint8_t i TSRMLS_DC);
inline int od_igbinary_serialize16(od_igbinary_serialize_data *igsd, uint16_t i TSRMLS_DC);
inline int od_igbinary_serialize32(od_igbinary_serialize_data *igsd, uint32_t i TSRMLS_DC);

inline int od_igbinary_serialize8_at(od_igbinary_serialize_data *igsd, uint8_t i, uint32_t pos TSRMLS_DC);
inline int od_igbinary_serialize16_at(od_igbinary_serialize_data *igsd, uint16_t i, uint32_t pos TSRMLS_DC);
inline int od_igbinary_serialize32_at(od_igbinary_serialize_data *igsd, uint32_t i, uint32_t pos TSRMLS_DC);

inline int od_igbinary_unserialize_get_key(od_igbinary_unserialize_data *igsd, char** key_p, uint32_t* key_len_p, long* key_index_p);

int od_igbinary_unserialize_zval(od_igbinary_unserialize_data *igsd, zval **z TSRMLS_DC);

inline int od_igbinary_unserialize_data_init(od_igbinary_unserialize_data *igsd TSRMLS_DC);

inline int od_igbinary_unserialize_header(od_igbinary_unserialize_data *igsd, uint32_t *header TSRMLS_DC);

inline int od_igbinary_unserialize_data_clone(od_igbinary_unserialize_data *dest, od_igbinary_unserialize_data *src TSRMLS_DC);

inline int od_igbinary_unserialize_init_string_table(od_igbinary_unserialize_data *igsd TSRMLS_DC);

inline void od_igbinary_unserialize_data_deinit(od_igbinary_unserialize_data *igsd TSRMLS_DC);

inline od_igbinary_type od_igbinary_get_type(od_igbinary_unserialize_data *igsd);

inline void od_igbinary_serialize_append_zero(od_igbinary_serialize_data *igsd);

inline int od_igbinary_serialize_memcpy(od_igbinary_serialize_data *igsd, uint8_t* s, uint32_t len);

inline int od_igbinary_serialize_skip_n(od_igbinary_serialize_data *igsd, int n TSRMLS_DC);

inline int od_igbinary_unserialize_skip_key(od_igbinary_unserialize_data *igsd);

inline int od_igbinary_serialize_value_len(od_igbinary_serialize_data *igsd, uint32_t len, uint32_t pos TSRMLS_DC);

inline int od_igbinary_serialize_array(od_igbinary_serialize_data *igsd, zval *z, zend_class_entry* ce, bool object, bool incomplete_class, bool in_od_serialize TSRMLS_DC);

inline int od_igbinary_serialize_string_table(od_igbinary_serialize_data *igsd TSRMLS_DC);

inline int od_igbinary_clone_string_table(od_igbinary_serialize_data *igsd, od_igbinary_unserialize_data *orig TSRMLS_DC);

inline int od_igbinary_serialize_update_string_table(od_igbinary_serialize_data *igsd, od_igbinary_unserialize_data *orig TSRMLS_DC);

int od_igbinary_serialize_zval(od_igbinary_serialize_data *igsd, zval *z TSRMLS_DC);

inline int od_igbinary_serialize_data_init(od_igbinary_serialize_data *igsd, bool scalar TSRMLS_DC);

inline void od_igbinary_serialize_data_deinit(od_igbinary_serialize_data *igsd TSRMLS_DC);

inline int od_igbinary_serialize_header(od_igbinary_serialize_data *igsd TSRMLS_DC);

inline int od_igbinary_serialize_long(od_igbinary_serialize_data *igsd, long l TSRMLS_DC);

inline int od_igbinary_serialize_string(od_igbinary_serialize_data *igsd, const char *s, uint32_t len TSRMLS_DC);

#endif
