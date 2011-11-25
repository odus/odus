/*
  +----------------------------------------------------------------------+
  | See COPYING file for further copyright information                   |
  +----------------------------------------------------------------------+ 
  | Author: Oleg Grenrus <oleg.grenrus@dynamoid.com>                     |
  | See CREDITS for contributors                                         |
  | Moidfied from od_hash.c                                              |
  | The hash key can be string or integer                                |
  +----------------------------------------------------------------------+ 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include "od_def.h"
#include "od_hash.h"
#include "od_debug.h"

#include "odwrapper.h"
extern zend_object_handlers od_wrapper_object_handlers;


inline static uint32_t od_nextpow2(uint32_t n) {
	uint32_t m = OD_HASH_DEFAULT_NUM;
	while (m < n) {
		m = m << 1;
	}

	return m;
}

int od_hash_init (ODHashTable **h_p, uint32_t size) {

	if(!h_p) {
		return FAILURE;
	}

	*h_p = (ODHashTable*) emalloc(sizeof(ODHashTable));

	ODHashTable* h = *h_p;

	if(!h) {
		return FAILURE;
	}

	size = od_nextpow2(size);

	h->size = size;
	h->used = 0;
	h->buckets = (ODBucket *) emalloc(sizeof(ODBucket) * size);
	if (h->buckets == NULL) {
		efree(*h_p);
		*h_p = NULL;
		return FAILURE;
	}

	memset(h->buckets, 0, sizeof(ODBucket) * size);
	
	return SUCCESS;
}

void od_hash_deinit (ODHashTable **h_p) {

	if(!h_p || !(*h_p) || !(*h_p)->buckets) return;

	uint32_t i;
	
	ODHashTable* h = *h_p;

	for (i = 0; i < h->size; i++) {
		OD_HASH_FREE_KEY(h->buckets[i]);
		OD_HASH_FREE_VAL(h->buckets[i]);
	}

	efree(h->buckets);

	h->size = 0;
	h->used = 0;

	efree((h_p));
}

inline static uint32_t _od_hash_find(ODHashTable *h, const char *key, uint32_t key_len, uint32_t hash) {
	uint32_t hv;
	uint32_t size;
	
	assert(h != NULL);
	
	size = h->size;
	hv = hash & (h->size-1);
	
	while (size > 0 &&
		OD_IS_OCCUPIED(h->buckets[hv]) &&
		(h->buckets[hv].hash != hash || h->buckets[hv].key_len != key_len  || (!(h->buckets[hv].key == NULL && key==NULL) && (h->buckets[hv].key[0]!=key[0] || memcmp(h->buckets[hv].key, key, key_len) != 0)))) {

#ifdef OD_DEBUG
		if(memcmp(TKEY,key,key_len)==0) {
			debug("try pos %u (key: %s len: %u hash: %u) for key: %s len: %u hash: %u", hv, h->buckets[hv].key,h->buckets[hv].key_len,h->buckets[hv].hash,key,key_len,hash);
		}
#endif

		hv = (hv + 1) & (h->size-1);
		size--;
	}
	
	return hv;
}

int od_hash_find(ODHashTable *h, const char *key, uint32_t key_len, uint32_t hash, ODBucket** ret_bkt) {
	uint32_t hv;

	assert(h != NULL);

	if(ret_bkt) {
		*ret_bkt = NULL;
	}

#ifdef OD_DEBUG
	check_od_ht(h,"before find");
#endif

	hv = _od_hash_find(h, key, key_len, hash);

#ifdef OD_DEBUG
	if(OD_IS_OCCUPIED(h->buckets[hv])) {
		debug("od_hash_find key '%s' at position %p bucket index: %d value: %s null",h->buckets[hv].key,h->buckets+hv,hv,h->buckets[hv].data == NULL?"is":"no");
	} else {
		if(memcmp(TKEY,key,key_len)==0) {
			//print_od_ht(h,TKEY);

			debug("pos: %d not find key '%s' len %u hash %u",hv,key,key_len,hash);
			web_debug("pos: %d not find key '%s' len %u hash %u",hv,key,key_len,hash);

			uint32_t size = h->size;
			uint32_t hv;

			hv = hash & (h->size-1);
			while(size>0  && OD_IS_OCCUPIED(h->buckets[hv])) {

				debug("check pos: %d key: '%s' len: %u hash: %u",hv,h->buckets[hv].key,h->buckets[hv].key_len,h->buckets[hv].hash);

				hv = (hv + 1) & (h->size-1);
				size--;
			}
		}
	}

	check_od_ht(h,"after find");
#endif



	if (!OD_IS_OCCUPIED(h->buckets[hv]) || h->buckets[hv].data == NULL) {
		if(ret_bkt) {
			*ret_bkt = h->buckets + hv;
		}
		return FAILURE;
	} else {
		if(ret_bkt) {
			*ret_bkt = h->buckets + hv;
		}
		return SUCCESS;
	}
}

#ifdef OD_DEBUG
void print_text(char* msg,FILE* fp) {
	fwrite(msg,1,strlen(msg),fp);
}

void check_od_ht(ODHashTable *h,char* msg) {
	if(!h) return;

	uint32_t i;

	ODBucket* bkt = h->buckets;

	for(i=0;i<h->size;i++) {

		if(!OD_IS_OCCUPIED(bkt[i]) && bkt[i].key!=NULL) {
			debug("msg: %s key '%s' has wrong flag %u [%d,%s,%s]", msg, bkt[i].key, bkt[i].flag,OD_LINE, OD_FUNCTION, OD_FILE);
			web_debug("msg: %s key '%s' has wrong flag %u [%d,%s,%s]", msg, bkt[i].key, bkt[i].flag,OD_LINE, OD_FUNCTION, OD_FILE);
		}
	}
}

void print_od_ht(ODHashTable *h,char* msg) {
	if(!h) return;

	FILE* fp = fopen("/tmp/hash","a+");

	if(!fp) return;

	fprintf(fp,"\nstart print hash table for %s table size: %u\n",msg,h->size);

	uint32_t i,k;

	ODBucket* bkt = h->buckets;

	zval* val;
	char buf[1024*2+5];
	char sbuf[512];
	char kbuf[512];
	char vbuf[512];
	char fbuf[512];

	for(i=0;i<h->size;i++) {
		if(OD_IS_OCCUPIED(bkt[i]) && bkt[i].data!=NULL) {

			val=(zval*)bkt[i].data;

			for(k=0;k<bkt[i].key_len;k++) {
				kbuf[k]=bkt[i].key[k];

				if(kbuf[k]==0) kbuf[k]='0';
			}
			kbuf[bkt[i].key_len]=0;

			if(val->type == IS_STRING) {
				int len = val->value.str.len<512?val->value.str.len:512;

				for(k=0;k<len;k++) {
					sbuf[k]=val->value.str.val[k];

					if(sbuf[k]==0) sbuf[k]='0';
				}

				sbuf[len]=0;

				sprintf(vbuf,"%s",sbuf);
			} else if(val->type == IS_LONG) {
				sprintf(vbuf,"%ld",val->value.lval);
			} else if(val->type == IS_BOOL) {
				sprintf(vbuf,"%s",val->value.lval>0?"yes":"no");
			} else if(val->type == IS_DOUBLE) {
				sprintf(vbuf,"%f",val->value.dval);
			} else if(val->type == IS_OBJECT) {
				sprintf(vbuf,"%s",IS_OD_WRAPPER(val)?"odwarpper":"normal object");
			} else if(val->type == IS_ARRAY) {
				sprintf(vbuf,"array",kbuf);
			} else {
				sprintf(vbuf,"other-type",kbuf);
			}

			sprintf(fbuf,"ocu: %d sle: %d mod:%d new:%d",OD_IS_OCCUPIED(bkt[i])?1:0, OD_IS_SLEEP(bkt[i])?1:0, OD_IS_MODIFIED(bkt[i])?1:0, OD_IS_NEW(bkt[i])?1:0);

			fprintf(fp,"pos: %d flag: (%x) %s hash: %u key: %s value: %s\n", i, bkt[i].flag, fbuf, bkt[i].hash, kbuf, vbuf);

		} else {
			if(OD_IS_OCCUPIED(bkt[i]) && bkt[i].data == NULL) {
				fprintf(fp,"pos: %d flag: (%x) hash: %u key: %s is null\n",i,bkt[i].flag, bkt[i].hash, bkt[i].key);
			} else {
				fprintf(fp,"pos: %d flag: (%x) hash: %u key: %s is not occupied value: %s null %s\n",i,bkt[i].flag, bkt[i].hash, bkt[i].key?bkt[i].key:"null", bkt[i].data?"not":"",bkt[i].key?"wrong":"");
			}
		}
	}

	fprintf(fp,"\nend print hash table for %s table size: %u\n",msg,h->size);

	fclose(fp);
}
#endif

inline static void od_hash_rehash(ODHashTable *h) {
	uint32_t hv;
	uint32_t i;
	ODHashTable* newh = NULL;
		
	assert(h != NULL);
	
	od_hash_init(&newh, h->size * 2);

	if(!newh) return;
	
	print_od_ht(h,"before rehash");

	for (i = 0; i < h->size; i++) {
		if (OD_IS_OCCUPIED(h->buckets[i])) {
			hv = _od_hash_find(newh, h->buckets[i].key, h->buckets[i].key_len, h->buckets[i].hash);
			newh->buckets[hv] = h->buckets[i];
		}
	}	
	

	print_od_ht(newh,"after rehash");

	efree(h->buckets);
	h->buckets = newh->buckets;
	h->size *= 2;

	efree(newh);
}


int od_hash_update (ODHashTable *h, const char *key, uint32_t key_len, uint32_t hash, uint8_t persist,void* data, ODBucket** ret_bkt) {
	uint32_t hv;

	debug("prepare update for key %s key_len: %d hash: %u",key,key_len,hash);

	if(ret_bkt) {
		*ret_bkt = NULL;
	}

	if ((((h->size>>2)<<1) + (h->size>>2)) < h->used + 1) {

#ifdef OD_DEBUG
		debug("rehash for key '%s'",key);
		check_od_ht(h,"before rehash");
#endif

		od_hash_rehash(h);

#ifdef OD_DEBUG
		check_od_ht(h,"after rehash");
#endif

	}
	
#ifdef OD_DEBUG
	check_od_ht(h,"update start");
#endif

	hv = _od_hash_find(h, key, key_len, hash);
	
	if (!OD_IS_OCCUPIED(h->buckets[hv])) {

		if(key==NULL){
			if(key_len!=0){
				od_error(E_ERROR,"integer key must have key_len==0 and key==NULL");
				return FAILURE;
			} else {
				h->buckets[hv].key=NULL;
				h->buckets[hv].key_len=0;
				h->buckets[hv].hash = hash;

				OD_RESET_PERSIST_KEY(h->buckets[hv]);
			}
		} else {
			if(persist) {
				h->buckets[hv].key = estrndup(key,key_len);
				if(h->buckets[hv].key==NULL) {
					return FAILURE;
				}
				OD_SET_PERSIST_KEY(h->buckets[hv]);
			} else {
				h->buckets[hv].key = key;
				OD_RESET_PERSIST_KEY(h->buckets[hv]);
			}

			h->buckets[hv].key_len = key_len;
			h->buckets[hv].hash = hash;
		}

		h->used++;

		OD_SET_OCCUPIED(h->buckets[hv]);
	}
	
	if(h->buckets[hv].data != data) {
		OD_HASH_FREE_VAL(h->buckets[hv]);
	}

	h->buckets[hv].data = data;

	if(ret_bkt) {
		*ret_bkt = h->buckets + hv;
	}

#ifdef OD_DEBUG
	//FIMXE
	char buf[1024];
	char kbuf[512];
	uint32_t i;

	for(i=0;i<key_len;i++) {
		kbuf[i]=key[i];

		if(kbuf[i]==0) kbuf[i]='0';
	}
	kbuf[key_len]=0;

	zval* val = (zval*)data;
	sprintf(buf,"update hash for key: %s with value: %ld hash: %u at pos %u with hash size %u flag: %x",kbuf,(val && val->type == IS_LONG)?val->value.lval:(-1234),hash,hv,h->size,h->buckets[hv].flag);

	if(memcmp(TKEY,key,key_len)==0) {
		print_od_ht(h,TKEY "update");
	}

	//print_od_ht(h,buf);

	debug(buf);

	check_od_ht(h,"update end");
#endif

	return SUCCESS;
}


int od_hash_remove (ODHashTable *h, const char *key, uint32_t key_len, uint32_t hash) {
	uint32_t hv;
	uint32_t j, k;

	assert(h != NULL);

	hv = _od_hash_find(h, key, key_len, hash);

	/* dont exists */
	if (!OD_IS_OCCUPIED(h->buckets[hv])) {
		return FAILURE;
	}

	h->used--;

	OD_HASH_FREE_KEY(h->buckets[hv]);
	OD_HASH_FREE_VAL(h->buckets[hv]);

	j = (hv + 1) & (h->size-1);
	while (OD_IS_OCCUPIED(h->buckets[j])) {
		k = h->buckets[j].hash & (h->size-1);

		if ((j > hv && (k <= hv || k > j)) || (j < hv && (k <= hv && k > j))) {
			h->buckets[hv] = h->buckets[j];
			hv = j;
		}
		j = (j + 1) & (h->size-1);
	}

	OD_HASH_RESET_BKT(h->buckets[hv]);

	return SUCCESS;
}
