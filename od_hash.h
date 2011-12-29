/*
  +----------------------------------------------------------------------+
  | See COPYING file for further copyright information                   |
  +----------------------------------------------------------------------+ 
  | Author: Oleg Grenrus <oleg.grenrus@dynamoid.com>                     |
  | See CREDITS for contributors                                         |
  +----------------------------------------------------------------------+ 
*/

#ifndef OD_HASH_H
#define OD_HASH_H

#include <assert.h>
#include <stdint.h>
#include <stddef.h>

#include "od_debug.h"

#define OD_HASH_DEFAULT_NUM 1<<2

#define OD_FLAG_MODIFIED 	0x01
#define OD_FLAG_NEW 		0x02
#define OD_FLAG_SLEEP 		0x04

#define OD_FLAG_PERSIST_KEY 0x40
#define OD_FLAG_OCCUPIED 	0x80

#define OD_SET_FLAG(flag,mask) (flag)=((flag)|(mask))
#define OD_RESET_FLAG(flag,mask) (flag)=((flag)&(~mask))
#define OD_TEST_FLAG(flag,mask) ((flag)&(mask))

#define OD_IS_MODIFIED(bkt) OD_TEST_FLAG((bkt).flag,OD_FLAG_MODIFIED)
#define OD_IS_NEW(bkt) OD_TEST_FLAG((bkt).flag,OD_FLAG_NEW)
#define OD_IS_SLEEP(bkt) OD_TEST_FLAG((bkt).flag,OD_FLAG_SLEEP)

#define OD_IS_PERSIST_KEY(bkt) OD_TEST_FLAG((bkt).flag,OD_FLAG_PERSIST_KEY)
#define OD_IS_OCCUPIED(bkt) OD_TEST_FLAG((bkt).flag,OD_FLAG_OCCUPIED)


#define OD_SET_MODIFIED(bkt) OD_SET_FLAG((bkt).flag,OD_FLAG_MODIFIED);
#define OD_SET_NEW(bkt) OD_SET_FLAG((bkt).flag,OD_FLAG_NEW);
#define OD_SET_SLEEP(bkt) OD_SET_FLAG((bkt).flag,OD_FLAG_SLEEP);

#define OD_SET_PERSIST_KEY(bkt) OD_SET_FLAG((bkt).flag,OD_FLAG_PERSIST_KEY)
#define OD_SET_OCCUPIED(bkt) OD_SET_FLAG((bkt).flag,OD_FLAG_OCCUPIED)

#define OD_RESET_MODIFIED(bkt) OD_RESET_FLAG((bkt).flag,OD_FLAG_MODIFIED)
#define OD_RESET_NEW(bkt) OD_RESET_FLAG((bkt).flag,OD_FLAG_NEW)
#define OD_RESET_SLEEP(bkt) OD_RESET_FLAG((bkt).flag,OD_FLAG_SLEEP)

#define OD_RESET_PERSIST_KEY(bkt) OD_RESET_FLAG((bkt).flag,OD_FLAG_PERSIST_KEY)
#define OD_RESET_OCCUPIED(bkt) OD_RESET_FLAG((bkt).flag,OD_FLAG_OCCUPIED)


#define OD_GET_HASH_VALUE(key,len) OD_HASH_VALUE(zend_get_hash_value(key,len+1))
#define OD_HASH_VALUE(hash) (0xFFFFFF & (hash))

#define OD_HASH_SIZE(h) ((h)?(h)->used:0)
#define OD_HASH_CAPACITY(h) ((h)?(h)->size:0)

#define OD_HASH_FREE_KEY(bkt) if ((bkt).key != NULL && OD_IS_PERSIST_KEY(bkt)) { efree((char*)((bkt).key)); (bkt).key = NULL; }
#define OD_HASH_FREE_VAL(bkt) OD_ZVAL_PTR_DTOR((zval**)(&(bkt).data)); (bkt).data = NULL;

#define OD_HASH_RESET_BKT(bkt) \
		(bkt).key = NULL; \
		(bkt).key_len = 0; \
		(bkt).flag = 0; \
		(bkt).hash = 0; \
		(bkt).data = NULL; \

typedef struct {
	const char* key;
	void* data;
	uint32_t key_len;
	uint32_t hash:24;
	uint8_t flag;
} ODBucket;

typedef struct {
	ODBucket* buckets;
	uint32_t size;
	uint32_t used;
} ODHashTable;

inline int od_hash_init (ODHashTable** h_p, uint32_t size);

inline void od_hash_deinit (ODHashTable** h_p);

inline int od_hash_update (ODHashTable *h, const char *key, uint32_t key_len, uint32_t hash, uint8_t persist, void* data, ODBucket** ret_bkt);

inline int od_hash_find (ODHashTable *h, const char *key, uint32_t key_len, uint32_t hash, ODBucket** ret_bkt);

inline int od_hash_remove (ODHashTable *h, const char *key, uint32_t key_len, uint32_t hash);

#ifdef OD_DEBUG
	void print_text(char* msg,FILE* fp);
	void print_od_ht(ODHashTable *h,char* msg);
	void check_od_ht(ODHashTable* h,char* msg);
#else
	#define print_text
	#define print_od_ht
	#define check_od_ht
#endif

#endif /* OD_HASH_H */
