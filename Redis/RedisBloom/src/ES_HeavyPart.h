#ifndef _ES_HEAVYPART_H_
#define _ES_HEAVYPART_H_

#include <stdint.h>  //  uint32_t
#include <stddef.h>  //  size_t
#include <stdbool.h> //  bool
#include <string.h>  //  memcpy
#include <stdlib.h>  //  calloc

//#define REDIS_MODULE_TARGET
#ifdef REDIS_MODULE_TARGET
#include "redismodule.h"
#define ES_CALLOC(count, size) RedisModule_Calloc(count, size)
#define ES_FREE(ptr) RedisModule_Free(ptr)
#else
#define ES_CALLOC(count, size) calloc(count, size)
#define ES_FREE(ptr) free(ptr)
#endif

#define COUNTER_PER_BUCKET 8
typedef uint32_t ES_HeavyPart_CounterType;
#define ES_HeavyPart_GetFlag(counter) (counter >> 31)
#define ES_HeavyPart_GetValue(counter) (counter & 0x7fffffff)

typedef struct ES_HeavyPart_Bucket
{
    char *key[COUNTER_PER_BUCKET];
    size_t key_len[COUNTER_PER_BUCKET];
    ES_HeavyPart_CounterType val[COUNTER_PER_BUCKET];
} ES_HeavyPart_Bucket;

typedef struct ES_HeavyPart
{
    uint32_t bucket_num;
    ES_HeavyPart_Bucket *buckets;
} ES_HeavyPart;

ES_HeavyPart *ES_HeavyPart_Create(uint32_t size);

void ES_HeavyPart_Destroy(ES_HeavyPart *hp);

void ES_HeavyPart_Clear(ES_HeavyPart *hp);

uint32_t ES_HeavyPart_Insert(ES_HeavyPart *hp, const char *key, size_t key_len, uint32_t f, char **swap_key, size_t *swap_key_len, ES_HeavyPart_CounterType *swap_val);

uint32_t ES_HeavyPart_QuickInsert(ES_HeavyPart *hp, const char *key, size_t key_len, uint32_t f);

ES_HeavyPart_CounterType ES_HeavyPart_Query(ES_HeavyPart *hp, const char *key, size_t key_len);

size_t ES_HeavyPart_Mem(ES_HeavyPart *hp);

#endif // _ES_LIGHTPART_H_
