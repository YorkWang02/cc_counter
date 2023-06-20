/*
 * Copyright 2019 Redis Ltd. and Contributors
 *
 * This file is available under the Redis Source Available License Agreement
 *
 * This Top-K Data Type is based on Heavy Keeper algorithm. The paper can be found
 * at https://www.usenix.org/system/files/conference/atc18/atc18-gong.pdf
 *
 * Implementation by Ariel Shtul
 */

#include <assert.h>  // assert
#include <math.h>    // q, ceil
#include <stdio.h>   // printf
#include <stdlib.h>  // malloc
#include <stdbool.h> // bool

#include "topk.h"
#include "../contrib/murmurhash2.h"

#define TOPK_HASH(item, itemlen, i) MurmurHash2(item, itemlen, i)
#define GA 1919

static inline uint32_t max(uint32_t a, uint32_t b) { return a > b ? a : b; }

static inline char *topKStrndup(const char *s, size_t n) {
    char *ret = TOPK_CALLOC(n + 1, sizeof(char));
    if (ret)
        memcpy(ret, s, n);
    return ret;
}

SSummary *SSummary_Create(uint32_t k) {
    assert(k > 0);

    SSummary *ssummary = TOPK_CALLOC(1, sizeof(SSummary));
    ssummary->K = k;
    ssummary->ID1_cnt = k;
    ssummary->ID2_cnt = k + 2;
    ssummary->ID1 = TOPK_CALLOC(k + 1, sizeof(uint32_t));
    ssummary->ID2 = TOPK_CALLOC(k + 3, sizeof(uint32_t));
    ssummary->head1 = TOPK_CALLOC(k, sizeof(uint32_t));
    ssummary->next1 = TOPK_CALLOC(k + 1, sizeof(uint32_t));
    ssummary->pre1 = TOPK_CALLOC(k + 1, sizeof(uint32_t));
    ssummary->head2 = TOPK_CALLOC(k + 2, sizeof(uint32_t));
    ssummary->next2 = TOPK_CALLOC(k + 3, sizeof(uint32_t));
    ssummary->pre2 = TOPK_CALLOC(k + 3, sizeof(uint32_t));
    ssummary->left = TOPK_CALLOC(k + 3, sizeof(uint32_t));
    ssummary->right = TOPK_CALLOC(k + 3, sizeof(uint32_t));
    ssummary->num = TOPK_CALLOC(k + 1, sizeof(counter_t));
    ssummary->data = TOPK_CALLOC(k + 1, sizeof(HeapBucket));

    for (uint32_t i = 1; i <= k; ++i)
        ssummary->ID1[i] = i;
    for (uint32_t i = 1; i <= k + 2; ++i)
        ssummary->ID2[i] = i;
    ssummary->min_ID2 = ssummary->ID2[ssummary->ID2_cnt--];
    ssummary->max_ID2 = ssummary->ID2[ssummary->ID2_cnt--];
    ssummary->right[ssummary->min_ID2] = ssummary->max_ID2;
    ssummary->left[ssummary->max_ID2] = ssummary->min_ID2;

    return ssummary;
}

void SSummary_Destroy(SSummary *ssummary) {
    assert(ssummary);

    for (uint32_t i = 1; i <= ssummary->K; ++i)
        if (ssummary->data[i].item)
            TOPK_FREE(ssummary->data[i].item);

    TOPK_FREE(ssummary->ID1);
    TOPK_FREE(ssummary->ID2);
    TOPK_FREE(ssummary->head1);
    TOPK_FREE(ssummary->next1);
    TOPK_FREE(ssummary->pre1);
    TOPK_FREE(ssummary->head2);
    TOPK_FREE(ssummary->next2);
    TOPK_FREE(ssummary->pre2);
    TOPK_FREE(ssummary->left);
    TOPK_FREE(ssummary->right);
    TOPK_FREE(ssummary->num);
    TOPK_FREE(ssummary->data);
    TOPK_FREE(ssummary);
}

uint32_t SSummary_FindID1(SSummary *ssummary, uint32_t fp, const char *item, size_t itemlen) {
    for (uint32_t i = ssummary->head1[fp % ssummary->K]; i; i = ssummary->next1[i]) {
        HeapBucket *tmp = ssummary->data + i;
        if (tmp->fp == fp && tmp->itemlen == itemlen && memcmp(tmp->item, item, itemlen) == 0)
            return i;
    }
    return 0;
}

void SSummary_DeleteMin(SSummary *ssummary) {
    assert(ssummary->ID1_cnt < ssummary->K);
    assert(ssummary->right[ssummary->min_ID2] != ssummary->max_ID2);

    uint32_t tmp_ID2 = ssummary->right[ssummary->min_ID2];
    uint32_t tmp_ID1 = ssummary->head2[tmp_ID2];

    ssummary->head2[tmp_ID2] = ssummary->next2[tmp_ID1];
    ssummary->pre2[ssummary->head2[tmp_ID2]] = 0;
    ssummary->ID1[++ssummary->ID1_cnt] = tmp_ID1;
    ssummary->num[tmp_ID1] = 0;
    ssummary->next1[ssummary->pre1[tmp_ID1]] = ssummary->next1[tmp_ID1];
    ssummary->pre1[ssummary->next1[tmp_ID1]] = ssummary->pre1[tmp_ID1];
    if (ssummary->head1[ssummary->data[tmp_ID1].fp % ssummary->K] == tmp_ID1)
        ssummary->head1[ssummary->data[tmp_ID1].fp % ssummary->K] = ssummary->next1[tmp_ID1];
    TOPK_FREE(ssummary->data[tmp_ID1].item);
    memset(&ssummary->data[tmp_ID1], 0, sizeof(HeapBucket));

    if (ssummary->head2[tmp_ID2] == 0) {
        ssummary->left[ssummary->right[tmp_ID2]] = ssummary->left[tmp_ID2];
        ssummary->right[ssummary->left[tmp_ID2]] = ssummary->right[tmp_ID2];
        ssummary->ID2[++ssummary->ID2_cnt] = tmp_ID2;
    }
}

counter_t SSummary_GetCountFromID2(SSummary *ssummary, uint32_t ID2) {
    assert(ssummary->head2[ID2]);
    return ssummary->data[ssummary->head2[ID2]].count;
}

void SSummary_Insert(SSummary *ssummary, uint32_t fp, counter_t count, const char *item,
                     size_t itemlen) {
    assert(ssummary->ID1_cnt);

    uint32_t tmp_ID1 = ssummary->ID1[ssummary->ID1_cnt--];
    uint32_t key = fp % ssummary->K;
    uint32_t next1_ID1 = ssummary->head1[key];
    ssummary->head1[key] = tmp_ID1;
    ssummary->next1[tmp_ID1] = next1_ID1;
    ssummary->pre1[tmp_ID1] = 0;
    ssummary->pre1[next1_ID1] = tmp_ID1;
    HeapBucket *tmp_ID1_data = ssummary->data + tmp_ID1;
    tmp_ID1_data->fp = fp;
    tmp_ID1_data->count = count;
    tmp_ID1_data->item = topKStrndup(item, itemlen);
    tmp_ID1_data->itemlen = itemlen;

    uint32_t left_ID2 = ssummary->min_ID2;
    uint32_t right_ID2 = ssummary->right[left_ID2];
    for (; SSummary_GetCountFromID2(ssummary, right_ID2) < count;
         left_ID2 = right_ID2, right_ID2 = ssummary->right[right_ID2])
        ;
    if (SSummary_GetCountFromID2(ssummary, right_ID2) == count) {
        uint32_t next2_ID1 = ssummary->head2[right_ID2];
        ssummary->head2[right_ID2] = tmp_ID1;
        ssummary->next2[tmp_ID1] = next2_ID1;
        ssummary->pre2[next2_ID1] = tmp_ID1;
        ssummary->pre2[tmp_ID1] = 0;
        ssummary->num[tmp_ID1] = right_ID2;
    } else {
        uint32_t tmp_ID2 = ssummary->ID2[ssummary->ID2_cnt--];
        ssummary->left[tmp_ID2] = left_ID2;
        ssummary->right[left_ID2] = tmp_ID2;
        ssummary->left[right_ID2] = tmp_ID2;
        ssummary->right[tmp_ID2] = right_ID2;
        ssummary->head2[tmp_ID2] = tmp_ID1;
        ssummary->next2[tmp_ID1] = ssummary->pre2[tmp_ID1] = 0;
        ssummary->num[tmp_ID1] = tmp_ID2;
    }
}

void SSummary_Add(SSummary *ssummary, uint32_t ID1, counter_t increment) {
    assert(ssummary);
    assert(ssummary->data[ID1].count);
    assert(increment > 0);

    uint32_t left_ID2 = ssummary->num[ID1];
    uint32_t right_ID2 = ssummary->right[left_ID2];

    ssummary->data[ID1].count += increment;
    ssummary->next2[ssummary->pre2[ID1]] = ssummary->next2[ID1];
    ssummary->pre2[ssummary->next2[ID1]] = ssummary->pre2[ID1];
    if (ssummary->head2[left_ID2] == ID1) {
        ssummary->head2[left_ID2] = ssummary->next2[ID1];
        if (ssummary->head2[left_ID2] == 0) {
            ssummary->left[ssummary->right[left_ID2]] = ssummary->left[left_ID2];
            ssummary->right[ssummary->left[left_ID2]] = ssummary->right[left_ID2];
            ssummary->ID2[++ssummary->ID2_cnt] = left_ID2;
            left_ID2 = ssummary->left[left_ID2];
        }
    }

    counter_t count = ssummary->data[ID1].count;
    for (; SSummary_GetCountFromID2(ssummary, right_ID2) < count;
         left_ID2 = right_ID2, right_ID2 = ssummary->right[right_ID2])
        ;

    if (SSummary_GetCountFromID2(ssummary, right_ID2) == count) {
        uint32_t next2_ID1 = ssummary->head2[right_ID2];
        ssummary->head2[right_ID2] = ID1;
        ssummary->next2[ID1] = next2_ID1;
        ssummary->pre2[next2_ID1] = ID1;
        ssummary->pre2[ID1] = 0;
        ssummary->num[ID1] = right_ID2;
    } else {
        uint32_t tmp_ID2 = ssummary->ID2[ssummary->ID2_cnt--];
        ssummary->left[tmp_ID2] = left_ID2;
        ssummary->right[left_ID2] = tmp_ID2;
        ssummary->left[right_ID2] = tmp_ID2;
        ssummary->right[tmp_ID2] = right_ID2;
        ssummary->head2[tmp_ID2] = ID1;
        ssummary->next2[ID1] = ssummary->pre2[ID1] = 0;
        ssummary->num[ID1] = tmp_ID2;
    }
}

void heapifyDown(HeapBucket *array, size_t len, size_t start) {
    size_t child = start;

    // check whether larger than children
    if (len < 2 || (len - 2) / 2 < child) {
        return;
    }
    child = 2 * child + 1;
    if ((child + 1) < len && (array[child].count > array[child + 1].count)) {
        ++child;
    }
    if (array[child].count > array[start].count) {
        return;
    }

    // swap while larger than child
    HeapBucket top = {0};
    memcpy(&top, &array[start], sizeof(HeapBucket));
    do {
        memcpy(&array[start], &array[child], sizeof(HeapBucket));
        start = child;

        if ((len - 2) / 2 < child) {
            break;
        }
        child = 2 * child + 1;

        if ((child + 1) < len && (array[child].count > array[child + 1].count)) {
            ++child;
        }
    } while (array[child].count < top.count);
    memcpy(&array[start], &top, sizeof(HeapBucket));
}

TopK *TopK_Create(uint32_t k, uint32_t width, uint32_t depth, double decay) {
    assert(k > 0);
    assert(width > 0);
    assert(depth > 0);
    assert(decay > 0 && decay <= 1);

    TopK *topk = (TopK *)TOPK_CALLOC(1, sizeof(TopK));
    topk->k = k;
    topk->width = width;
    topk->depth = depth;
    topk->decay = decay;
    topk->data = TOPK_CALLOC(((size_t)width) * depth, sizeof(Bucket));
    topk->heap = TOPK_CALLOC(k, sizeof(HeapBucket));

    for (uint32_t i = 0; i < TOPK_DECAY_LOOKUP_TABLE; ++i) {
        topk->lookupTable[i] = pow(decay, i);
    }

    return topk;
}

void TopK_Destroy(TopK *topk) {
    assert(topk);

    for (uint32_t i = 0; i < topk->k; ++i) {
        TOPK_FREE(topk->heap[i].item);
    }

    TOPK_FREE(topk->heap);
    topk->heap = NULL;
    TOPK_FREE(topk->data);
    topk->data = NULL;
    TOPK_FREE(topk);
}

// Complexity O(k + strlen)
static HeapBucket *checkExistInHeap(TopK *topk, const char *item, size_t itemlen) {
    uint32_t fp = TOPK_HASH(item, itemlen, GA);
    HeapBucket *runner = topk->heap;

    for (int32_t i = topk->k - 1; i >= 0; --i)
        if (fp == (runner + i)->fp && itemlen == (runner + i)->itemlen &&
            memcmp((runner + i)->item, item, itemlen) == 0) {
            return runner + i;
        }
    return NULL;
}

char *TopK_Add(TopK *topk, const char *item, size_t itemlen, uint32_t increment) {
    assert(topk);
    assert(item);
    assert(itemlen);

    bool flag = false;
    HeapBucket *itemHeapPtr = checkExistInHeap(topk, item, itemlen);
    if (itemHeapPtr)
        flag = true;

    Bucket *runner;
    counter_t *countPtr;
    counter_t maxCount = 0;
    uint32_t fp = TOPK_HASH(item, itemlen, GA);

    counter_t heapMin = topk->heap->count;

    // get max item count
    for (uint32_t i = 0; i < topk->depth; ++i) {
        uint32_t loc = TOPK_HASH(item, itemlen, i) % topk->width;
        runner = topk->data + i * topk->width + loc;
        countPtr = &runner->count;

        if (runner->fp == fp) {
            if (flag || *countPtr <= heapMin)
                *countPtr += 1;
            maxCount = max(maxCount, *countPtr);
        } else {
            uint32_t local_incr = increment;
            for (; local_incr > 0; --local_incr) {
                double decay;
                if (*countPtr < TOPK_DECAY_LOOKUP_TABLE) {
                    decay = topk->lookupTable[*countPtr];
                } else {
                    //  using precalculate lookup table to save cpu
                    decay = pow(topk->lookupTable[TOPK_DECAY_LOOKUP_TABLE - 1],
                                (*countPtr / TOPK_DECAY_LOOKUP_TABLE) *
                                    topk->lookupTable[*countPtr % TOPK_DECAY_LOOKUP_TABLE]);
                }
                double chance = rand() / (double)RAND_MAX;
                if (chance < decay) {
                    if (*countPtr <= 1) {
                        runner->fp = fp;
                        *countPtr = local_incr;
                        maxCount = max(maxCount, *countPtr);
                        break;
                    } else
                        --*countPtr;
                }
            }
        }

        /*
        if (*countPtr == 0) {
            runner->fp = fp;
            *countPtr = increment;
            maxCount = max(maxCount, *countPtr);
        } else if (runner->fp == fp) {
            *countPtr += increment;
            maxCount = max(maxCount, *countPtr);
        } else {
            uint32_t local_incr = increment;
            for (; local_incr > 0; --local_incr) {
                double decay;
                if (*countPtr < TOPK_DECAY_LOOKUP_TABLE) {
                    decay = topk->lookupTable[*countPtr];
                } else {
                    //  using precalculate lookup table to save cpu
                    decay = pow(topk->lookupTable[TOPK_DECAY_LOOKUP_TABLE - 1],
                                (*countPtr / TOPK_DECAY_LOOKUP_TABLE) *
                                    topk->lookupTable[*countPtr % TOPK_DECAY_LOOKUP_TABLE]);
                }
                double chance = rand() / (double)RAND_MAX;
                if (chance < decay) {
                    --*countPtr;
                    if (*countPtr == 0) {
                        runner->fp = fp;
                        *countPtr = local_incr;
                        maxCount = max(maxCount, *countPtr);
                    }
                }
            }
        }
        */
    }

    if (!flag) {
        if (maxCount - heapMin == 1 || heapMin == 0) {
            char *expelled = topk->heap[0].item;

            topk->heap[0].count = maxCount;
            topk->heap[0].fp = fp;
            topk->heap[0].item = topKStrndup(item, itemlen);
            topk->heap[0].itemlen = itemlen;
            heapifyDown(topk->heap, topk->k, 0);
            return expelled;
        }
    } else if (maxCount > itemHeapPtr->count) {
        itemHeapPtr->count = maxCount; // Not max of the two, as it might have been decayed
        heapifyDown(topk->heap, topk->k, itemHeapPtr - topk->heap);
    }

    /*
    // update heap
    if (maxCount >= heapMin) {
        HeapBucket *itemHeapPtr = checkExistInHeap(topk, item, itemlen);
        if (itemHeapPtr != NULL) {
            itemHeapPtr->count = maxCount; // Not max of the two, as it might have been decayed
            heapifyDown(topk->heap, topk->k, itemHeapPtr - topk->heap);
        } else {
            // TOPK_FREE(topk->heap[0].item);
            char *expelled = topk->heap[0].item;

            topk->heap[0].count = maxCount;
            topk->heap[0].fp = fp;
            topk->heap[0].item = topKStrndup(item, itemlen);
            topk->heap[0].itemlen = itemlen;
            heapifyDown(topk->heap, topk->k, 0);
            return expelled;
        }
    }
    */
    return NULL;
}

bool TopK_Query(TopK *topk, const char *item, size_t itemlen) {
    return checkExistInHeap(topk, item, itemlen) != NULL;
}

size_t TopK_Count(TopK *topk, const char *item, size_t itemlen) {
    assert(topk);
    assert(item);
    assert(itemlen);

    Bucket *runner = NULL;
    uint32_t fp = TOPK_HASH(item, itemlen, GA);
    // TODO: The optimization of >heapMin should be revisited for performance
    counter_t heapMin = topk->heap->count;
    HeapBucket *heapPtr = checkExistInHeap(topk, item, itemlen);
    counter_t res = 0;

    for (uint32_t i = 0; i < topk->depth; ++i) {
        uint32_t loc = TOPK_HASH(item, itemlen, i) % topk->width;
        runner = topk->data + i * topk->width + loc;
        if (runner->fp == fp && (heapPtr == NULL || runner->count >= heapMin)) {
            res = max(res, runner->count);
        }
    }
    return res;
}

int cmpHeapBucket(const void *tmp1, const void *tmp2) {
    const HeapBucket *res1 = tmp1;
    const HeapBucket *res2 = tmp2;
    return res1->count < res2->count ? 1 : res1->count > res2->count ? -1 : 0;
}

HeapBucket *TopK_List(TopK *topk) {
    HeapBucket *heapList = TOPK_CALLOC(topk->k, (sizeof(*heapList)));
    memcpy(heapList, topk->heap, topk->k * sizeof(HeapBucket));
    qsort(heapList, topk->k, sizeof(*heapList), cmpHeapBucket);
    return heapList;
}
