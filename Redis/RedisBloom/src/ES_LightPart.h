#ifndef _ES_LIGHTPART_H_
#define _ES_LIGHTPART_H_

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

typedef uint8_t ES_LightPart_CounterType;
#define ES_LightPart_CounterMax 255

typedef struct ES_LightPart
{
    int counter_num;
    ES_LightPart_CounterType *counters;
} ES_LightPart;

ES_LightPart *ES_LightPart_Create(uint32_t size);

void ES_LightPart_Destroy(ES_LightPart *lp);

void ES_LightPart_Clear(ES_LightPart *lp);

void ES_LightPart_Insert(ES_LightPart *lp, const char *key, size_t key_len, uint32_t f);

void ES_LightPart_SwapInsert(ES_LightPart *lp, const char *key, size_t key_len, uint32_t f);

ES_LightPart_CounterType ES_LightPart_Query(ES_LightPart *lp, const char *key, size_t key_len);

size_t ES_LightPart_Mem(ES_LightPart *lp);

#endif // _ES_LIGHTPART_H_
