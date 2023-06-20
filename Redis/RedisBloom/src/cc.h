#ifndef RM_CCS_H
#define RM_CCS_H

#include <stdint.h> // uint32_t

//#define REDIS_MODULE_TARGET
#ifdef REDIS_MODULE_TARGET
#include "redismodule.h"
#define CCS_CALLOC(count, size) RedisModule_Calloc(count, size)
#define CCS_FREE(ptr) RedisModule_Free(ptr)
#else
#define CCS_CALLOC(count, size) calloc(count, size)
#define CCS_FREE(ptr) free(ptr)
#endif

#define CC_d 2
#define BN 4
#define MAX_MEM 100000

typedef struct node_ {
    uint32_t C, FP;
} node;

typedef struct CC {
    size_t width;
    size_t depth;
    node HK[CC_d][MAX_MEM+10][BN];
    //size_t counter;
} CCSketch;

/*typedef struct {
    CMSketch *dest;
    long long numKeys;
    CMSketch **cmsArray;
    long long *weights;
} mergeParams;*/

/* Creates a new Count-Min Sketch with dimensions of width * depth */
CCSketch *NewCCSketch(size_t width, size_t depth);

/*  Recommends width & depth for expected n different items,
    with probability of an error  - prob and over estimation
    error - overEst (use 1 for max accuracy) */
//void CMS_DimFromProb(double overEst, double prob, size_t *width, size_t *depth);

void CCS_Destroy(CCSketch *cc);

/*  Increases item count in value.
    Value must be a non negative number */
size_t CCS_IncrBy(CCSketch *cc, const char *item, size_t strlen, size_t value);

/* Returns an estimate counter for item */
size_t CCS_Query(CCSketch *cc, const char *item, size_t strlen);

/*  Merges multiple CMSketches into a single one.
    All sketches must have identical width and depth.
    dest must be already initialized.
*/
//void CMS_Merge(CMSketch *dest, size_t quantity, const CMSketch **src, const long long *weights);
//void CMS_MergeParams(mergeParams params);

/* Help function */
//void CMS_Print(const CMSketch *cms);

#endif
