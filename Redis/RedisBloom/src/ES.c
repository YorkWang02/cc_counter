#include <assert.h>  // assert
#include <math.h>    // q, ceil
#include <stdio.h>   // printf
#include <stdlib.h>  // malloc
#include <stdbool.h> // bool

#include "ES.h"
#include "murmurhash2.h"

ES *ES_Create(uint32_t heavy_size, uint32_t light_size)
{
    assert(heavy_size);
    assert(light_size);

    ES *es = (ES *)ES_CALLOC(1, sizeof(ES));
    es->hp = ES_HeavyPart_Create(heavy_size);
    es->lp = ES_LightPart_Create(light_size);

    return es;
}

void ES_Destroy(ES *es)
{
    assert(es);

    ES_HeavyPart_Destroy(es->hp);
    ES_LightPart_Destroy(es->lp);
    ES_FREE(es);
}

void ES_Clear(ES *es)
{
    assert(es);

    ES_HeavyPart_Clear(es->hp);
    ES_LightPart_Clear(es->lp);
}

void ES_Insert(ES *es, const char *key, size_t key_len, uint32_t f)
{
    assert(es);
    assert(key);
    assert(key_len);

    char *swap_key;
    size_t swap_key_len;
    ES_HeavyPart_CounterType swap_val = 0;
    uint32_t result = ES_HeavyPart_Insert(es->hp, key, key_len, f, &swap_key, &swap_key_len, &swap_val);
    switch (result)
    {
    case 0:
        return;
    case 1:
    {
        if (ES_HeavyPart_GetFlag(swap_val))
            ES_LightPart_Insert(es->lp, swap_key, swap_key_len, ES_HeavyPart_GetValue(swap_val));
        else
            ES_LightPart_SwapInsert(es->lp, swap_key, swap_key_len, ES_HeavyPart_GetValue(swap_val));
        return;
    }
    case 2:
        ES_LightPart_Insert(es->lp, key, key_len, 1);
        return;
    default:
        printf("error return value !\n");
        exit(1);
    }
}

void ES_QuickInsert(ES *es, const char *key, size_t key_len, uint32_t f)
{
    assert(es);
    assert(key);
    assert(key_len);

    ES_HeavyPart_QuickInsert(es->hp, key, key_len, f);
}

uint32_t ES_Query(ES *es, const char *key, size_t key_len)
{
    assert(es);
    assert(key);
    assert(key_len);

    ES_HeavyPart_CounterType heavy_result = ES_HeavyPart_Query(es->hp, key, key_len);
    if (heavy_result == 0 || ES_HeavyPart_GetFlag(heavy_result))
    {
        ES_LightPart_CounterType light_result = ES_LightPart_Query(es->lp, key, key_len);
        return (uint32_t)ES_HeavyPart_GetValue(heavy_result) + light_result;
    }
    return heavy_result;
}

size_t ES_MEM(ES *es)
{
    assert(es);

    return sizeof(ES_LightPart) + sizeof(ES_HeavyPart) + ES_LightPart_Mem(es->lp) + ES_HeavyPart_Mem(es->hp);
}