#include <assert.h>  // assert
#include <math.h>    // q, ceil
#include <stdio.h>   // printf
#include <stdlib.h>  // malloc
#include <stdbool.h> // bool

#include "ES_LightPart.h"
#include "murmurhash2.h"

#define SEED_LP 1000
#define ES_HASH(item, itemlen, i) MurmurHash2(item, itemlen, i)

ES_LightPart *ES_LightPart_Create(uint32_t size)
{
    assert(size > 0);

    ES_LightPart *lp = (ES_LightPart *)ES_CALLOC(1, sizeof(ES_LightPart));
    lp->counter_num = size;
    lp->counters = (ES_LightPart_CounterType *)ES_CALLOC(size, sizeof(ES_LightPart_CounterType));

    return lp;
}

void ES_LightPart_Destroy(ES_LightPart *lp)
{
    assert(lp);
    assert(lp->counters);

    ES_FREE(lp->counters);
    ES_FREE(lp);
}

void ES_LightPart_Clear(ES_LightPart *lp)
{
    assert(lp);
    assert(lp->counters);

    memset(lp->counters, 0, lp->counter_num * sizeof(ES_LightPart_CounterType));
}

void ES_LightPart_Insert(ES_LightPart *lp, const char *key, size_t key_len, uint32_t f)
{
    assert(lp);
    assert(key);
    assert(key_len);
    assert(f <= ES_LightPart_CounterMax);

    ES_LightPart_CounterType *counter = &(lp->counters[ES_HASH(key, key_len, SEED_LP) % lp->counter_num]);

    if (*counter > ES_LightPart_CounterMax - f)
        *counter = ES_LightPart_CounterMax;
    else
        *counter += f;
}

void ES_LightPart_SwapInsert(ES_LightPart *lp, const char *key, size_t key_len, uint32_t f)
{
    assert(lp);
    assert(key);
    assert(key_len);
    assert(f <= ES_LightPart_CounterMax);

    lp->counters[ES_HASH(key, key_len, SEED_LP) % lp->counter_num] = f;
}

ES_LightPart_CounterType ES_LightPart_Query(ES_LightPart *lp, const char *key, size_t key_len)
{
    assert(lp);
    assert(key);
    assert(key_len);

    return lp->counters[ES_HASH(key, key_len, SEED_LP) % lp->counter_num];
}

size_t ES_LightPart_Mem(ES_LightPart *lp)
{
    return lp->counter_num * sizeof(ES_LightPart_CounterType);
}