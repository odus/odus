
#ifndef _OD_DEBUG_H_
#define _OD_DEBUG_H_

struct _zend_property_info *od_get_property_info(zend_class_entry *ce, zval *member, int silent TSRMLS_DC);

#define OD_CREATE_FATAL do { \
	int*p = NULL; \
	*p=123; \
	} while(0)


//#define OD_DEBUG

#define OD_DEBUG_MEM 0
#define OD_DEBUG_TIME 0
#define OD_DEBUG_BUFFER 0

#ifdef OD_DEBUG
void debug(char* format, ...);

void debug_mem(char* format, ...);

void debug_buffer(uint8_t* buffer, uint32_t len, uint32_t pos);

void print_ht(HashTable* ht);
#else

#define debug
#define debug_mem

#if OD_DEBUG_BUFFER
void debug_buffer(uint8_t* buffer, uint32_t len, uint32_t pos);
#else
#define debug_buffer
#endif

#define print_ht

#endif

#endif//_OD_DEBUG_H_
