#ifndef _ES_H_
#define _ES_H_

#include <stdint.h>  //  uint32_t
#include <stddef.h>  //  size_t
#include <stdbool.h> //  bool
#include <string.h>  //  memcpy
#include <stdlib.h>  //  calloc

#include "ES_HeavyPart.h"
#include "ES_LightPart.h"

//#define REDIS_MODULE_TARGET
#ifdef REDIS_MODULE_TARGET
#include "redismodule.h"
#define ES_CALLOC(count, size) RedisModule_Calloc(count, size)
#define ES_FREE(ptr) RedisModule_Free(ptr)
#else
#define ES_CALLOC(count, size) calloc(count, size)
#define ES_FREE(ptr) free(ptr)
#endif

typedef struct ES
{
    ES_HeavyPart *hp;
    ES_LightPart *lp;
} ES;

ES *ES_Create(uint32_t heavy_size, uint32_t light_size);

void ES_Destroy(ES *es);

void ES_Clear(ES *es);

void ES_Insert(ES *es, const char *key, size_t key_len, uint32_t f);

void ES_QuickInsert(ES *es, const char *key, size_t key_len, uint32_t f);

uint32_t ES_Query(ES *es, const char *key, size_t key_len);

size_t ES_MEM(ES *es);

#endif