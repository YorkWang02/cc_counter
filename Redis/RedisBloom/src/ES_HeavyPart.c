#include <assert.h>  // assert
#include <math.h>    // q, ceil
#include <stdio.h>   // printf
#include <stdlib.h>  // malloc
#include <stdbool.h> // bool

#include "ES_HeavyPart.h"
#include "murmurhash2.h"

#define SEED_HP 1001
#define ES_HASH(item, itemlen, i) MurmurHash2(item, itemlen, i)

static inline char *ES_Strndup(const char *s, size_t n)
{
    char *ret = ES_CALLOC(n + 1, sizeof(char));
    if (ret)
        memcpy(ret, s, n);
    return ret;
}

ES_HeavyPart *ES_HeavyPart_Create(uint32_t size)
{
    assert(size);

    ES_HeavyPart *hp = (ES_HeavyPart *)ES_CALLOC(1, sizeof(ES_HeavyPart));
    hp->bucket_num = size;
    hp->buckets = (ES_HeavyPart_Bucket *)ES_CALLOC(size, sizeof(ES_HeavyPart_Bucket));

    return hp;
}

void ES_HeavyPart_Destroy(ES_HeavyPart *hp)
{
    assert(hp);
    assert(hp->buckets);

    for (uint32_t i = 0; i < hp->bucket_num; ++i)
        for (uint32_t j = 0; j < COUNTER_PER_BUCKET; ++j)
            if (hp->buckets[i].key[j])
                ES_FREE(hp->buckets[i].key[j]);

    ES_FREE(hp->buckets);
    ES_FREE(hp);
}

void ES_HeavyPart_Clear(ES_HeavyPart *hp)
{
    assert(hp);
    assert(hp->buckets);

    for (uint32_t i = 0; i < hp->bucket_num; ++i)
        for (uint32_t j = 0; j < COUNTER_PER_BUCKET; ++j)
        {
            hp->buckets[i].key_len[j] = 0;
            hp->buckets[i].val[j] = 0;
            if (hp->buckets[i].key[j])
                ES_FREE(hp->buckets[i].key[j]);
        }
}

uint32_t ES_HeavyPart_Insert(ES_HeavyPart *hp, const char *key, size_t key_len, uint32_t f, char **swap_key, size_t *swap_key_len, ES_HeavyPart_CounterType *swap_val)
{
    assert(hp);
    assert(key);
    assert(key_len);

    uint32_t pos = ES_HASH(key, key_len, SEED_HP) % hp->bucket_num;
    int matched = -1, empty = -1, min_counter = 0;
    uint32_t min_counter_val = ES_HeavyPart_GetValue(hp->buckets[pos].val[0]);
    for (uint32_t i = 0; i < COUNTER_PER_BUCKET - 1; ++i)
    {
        if (key_len == hp->buckets[pos].key_len[i] && strcmp(hp->buckets[pos].key[i], key) == 0)
        {
            matched = i;
            break;
        }
        if (hp->buckets[pos].key[i] == 0 && empty == -1)
            empty = i;
        if (min_counter_val > ES_HeavyPart_GetValue(hp->buckets[pos].val[i]))
        {
            min_counter = i;
            min_counter_val = ES_HeavyPart_GetValue(hp->buckets[pos].val[i]);
        }
    }

    /* if matched */
    if (matched != -1)
    {
        hp->buckets[pos].val[matched] += f;
        return 0;
    }

    /* if there has empty bucket */
    if (empty != -1)
    {
        hp->buckets[pos].key[empty] = ES_Strndup(key, key_len);
        hp->buckets[pos].key_len[empty] = key_len;
        hp->buckets[pos].val[empty] = f;
        return 0;
    }

    /* update guard val and comparison */
    uint32_t guard_val = hp->buckets[pos].val[COUNTER_PER_BUCKET - 1];
    guard_val += 1;

    if (guard_val <= (min_counter_val << 3))
    {
        hp->buckets[pos].val[COUNTER_PER_BUCKET - 1] = guard_val;
        return 2;
    }

    *swap_key = hp->buckets[pos].key[min_counter];
    *swap_key_len = hp->buckets[pos].key_len[min_counter];
    *swap_val = hp->buckets[pos].val[min_counter];

    hp->buckets[pos].val[COUNTER_PER_BUCKET - 1] = 0;
    hp->buckets[pos].key[min_counter] = ES_Strndup(key, key_len);
    hp->buckets[pos].key_len[min_counter] = key_len;
    hp->buckets[pos].val[min_counter] = 0x80000001;

    return 1;
}

uint32_t ES_HeavyPart_QuickInsert(ES_HeavyPart *hp, const char *key, size_t key_len, uint32_t f)
{
    assert(hp);
    assert(key);
    assert(key_len);

    uint32_t pos = ES_HASH(key, key_len, SEED_HP) % hp->bucket_num;
    int matched = -1, empty = -1, min_counter = 0;
    uint32_t min_counter_val = ES_HeavyPart_GetValue(hp->buckets[pos].val[0]);
    for (uint32_t i = 0; i < COUNTER_PER_BUCKET - 1; ++i)
    {
        if (key_len == hp->buckets[pos].key_len[i] && strcmp(hp->buckets[pos].key[i], key) == 0)
        {
            matched = i;
            break;
        }
        if (hp->buckets[pos].key[i] == 0 && empty == -1)
            empty = i;
        if (min_counter_val > ES_HeavyPart_GetValue(hp->buckets[pos].val[i]))
        {
            min_counter = i;
            min_counter_val = ES_HeavyPart_GetValue(hp->buckets[pos].val[i]);
        }
    }

    /* if matched */
    if (matched != -1)
    {
        hp->buckets[pos].val[matched] += f;
        return 0;
    }

    /* if there has empty bucket */
    if (empty != -1)
    {
        hp->buckets[pos].key[empty] = ES_Strndup(key, key_len);
        hp->buckets[pos].key_len[empty] = key_len;
        hp->buckets[pos].val[empty] = f;
        return 0;
    }

    /* update guard val and comparison */
    uint32_t guard_val = hp->buckets[pos].val[COUNTER_PER_BUCKET - 1];
    guard_val += 1;

    if (guard_val <= (min_counter_val << 3))
    {
        hp->buckets[pos].val[COUNTER_PER_BUCKET - 1] = guard_val;
        return 2;
    }

    hp->buckets[pos].val[COUNTER_PER_BUCKET - 1] = 0;
    hp->buckets[pos].key[min_counter] = ES_Strndup(key, key_len);
    hp->buckets[pos].key_len[min_counter] = key_len;

    return 1;
}

ES_HeavyPart_CounterType ES_HeavyPart_Query(ES_HeavyPart *hp, const char *key, size_t key_len)
{
    assert(hp);
    assert(key);
    assert(key_len);

    uint32_t pos = ES_HASH(key, key_len, SEED_HP) % hp->bucket_num;

    for (int i = 0; i < COUNTER_PER_BUCKET - 1; ++i)
        if (key_len == hp->buckets[pos].key_len[i] && strcmp(key, hp->buckets[pos].key[i]) == 0)
            return hp->buckets[pos].val[i];
    return 0;
}

size_t ES_HeavyPart_Mem(ES_HeavyPart *hp)
{
    assert(hp);
    return hp->bucket_num * sizeof(ES_HeavyPart_Bucket);
}