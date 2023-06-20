//#include <math.h>     ceil, log10f
//#include <strings.h>  strncasecmp
#include <assert.h>

#include "version.h"
#include "rmutil/util.h"

#include "ES.h"
#include "rm_ES.h"

#define INNER_ERROR(x)                                                                             \
    RedisModule_ReplyWithError(ctx, x);                                                            \
    return REDISMODULE_ERR;

RedisModuleType *ESType;

static int GetESKey(RedisModuleCtx *ctx, RedisModuleString *keyName, ES **es, int mode) {
    // All using this function should call RedisModule_AutoMemory to prevent memory leak
    RedisModuleKey *key = RedisModule_OpenKey(ctx, keyName, mode);
    if (RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY) {
        RedisModule_CloseKey(key);
        INNER_ERROR("ES: key does not exist");
    } else if (RedisModule_ModuleTypeGetType(key) != ESType) {
        RedisModule_CloseKey(key);
        INNER_ERROR(REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    *es = RedisModule_ModuleTypeGetValue(key);
    RedisModule_CloseKey(key);
    return REDISMODULE_OK;
}

static int createES(RedisModuleCtx *ctx, RedisModuleString **argv, int argc, ES **es) {
    long long heavy_size, light_size;
    if (argc == 4) {
        if ((RedisModule_StringToLongLong(argv[3], &heavy_size) != REDISMODULE_OK) ||
            heavy_size < 1) {
            INNER_ERROR("ES: invalid heavy part size");
        }
        if ((RedisModule_StringToLongLong(argv[4], &light_size) != REDISMODULE_OK) ||
            light_size < 1) {
            INNER_ERROR("ES: invalid light part size");
        }
    } else {
        heavy_size = 2400;
        light_size = 460800;
    }
    *es = ES_Create(heavy_size, light_size);
    return REDISMODULE_OK;
}

static int ES_Create_Cmd(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc != 2 && argc != 4) {
        return RedisModule_WrongArity(ctx);
    }

    RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    if (RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_EMPTY) {
        RedisModule_ReplyWithError(ctx, "ES: key already exists");
        goto final;
    }

    ES *es = NULL;
    if (createES(ctx, argv, argc, &es) != REDISMODULE_OK)
        goto final;

    if (RedisModule_ModuleTypeSetValue(key, ESType, es) == REDISMODULE_ERR) {
        goto final;
    }

    RedisModule_ReplicateVerbatim(ctx);
    RedisModule_ReplyWithSimpleString(ctx, "OK");
final:
    RedisModule_CloseKey(key);
    return REDISMODULE_OK;
}

static int ES_Insert_Cmd(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc < 3)
        return RedisModule_WrongArity(ctx);

    ES *es;
    if (GetESKey(ctx, argv[1], &es, REDISMODULE_READ | REDISMODULE_WRITE) != REDISMODULE_OK) {
        return REDISMODULE_OK;
    }

    int itemCount = argc - 2;
    RedisModule_ReplyWithArray(ctx, itemCount);

    for (int i = 0; i < itemCount; ++i) {
        size_t itemlen;
        const char *item = RedisModule_StringPtrLen(argv[i + 2], &itemlen);
        ES_Insert(es, item, itemlen, 1);

        RedisModule_ReplyWithNull(ctx);
    }
    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}

static int ES_QuickInsert_Cmd(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc < 3)
        return RedisModule_WrongArity(ctx);

    ES *es;
    if (GetESKey(ctx, argv[1], &es, REDISMODULE_READ | REDISMODULE_WRITE) != REDISMODULE_OK) {
        return REDISMODULE_OK;
    }

    int itemCount = argc - 2;
    RedisModule_ReplyWithArray(ctx, itemCount);

    for (int i = 0; i < itemCount; ++i) {
        size_t itemlen;
        const char *item = RedisModule_StringPtrLen(argv[i + 2], &itemlen);
        ES_QuickInsert(es, item, itemlen, 1);

        RedisModule_ReplyWithNull(ctx);
    }
    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}

static int ES_Query_Cmd(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc < 3)
        return RedisModule_WrongArity(ctx);

    ES *es;
    if (GetESKey(ctx, argv[1], &es, REDISMODULE_READ) != REDISMODULE_OK)
        return REDISMODULE_ERR;

    size_t itemlen;
    long long res;
    RedisModule_ReplyWithArray(ctx, argc - 2);
    for (int i = 2; i < argc; ++i) {
        const char *item = RedisModule_StringPtrLen(argv[i], &itemlen);
        res = ES_Query(es, item, itemlen);
        RedisModule_ReplyWithLongLong(ctx, res);
    }

    return REDISMODULE_OK;
}

/**************** Module functions *********************************/

static void ESRdbSave(RedisModuleIO *io, void *obj) {
    ES *es = obj;

    RedisModule_SaveStringBuffer(io, (const char *)es->hp, sizeof(ES_HeavyPart));
    RedisModule_SaveUnsigned(io, es->hp->bucket_num);
    RedisModule_SaveStringBuffer(io, (const char *)es->hp->buckets,
                                 ((size_t)es->hp->bucket_num) * sizeof(ES_HeavyPart_Bucket));
    for (uint32_t i = 0; i < es->hp->bucket_num; ++i)
        for (uint32_t j = 0; j < COUNTER_PER_BUCKET; ++j) {
            if (es->hp->buckets[i].key[j] != NULL) {
                RedisModule_SaveStringBuffer(io, es->hp->buckets[i].key[j],
                                             es->hp->buckets[i].key_len[j] + 1);
            } else {
                RedisModule_SaveStringBuffer(io, "", 1);
            }
        }

    RedisModule_SaveStringBuffer(io, (const char *)es->lp, sizeof(ES_LightPart));
    RedisModule_SaveStringBuffer(io, (const char *)es->lp->counters,
                                 ((size_t)es->lp->counter_num) * sizeof(ES_LightPart_CounterType));
}

static void *ESRdbLoad(RedisModuleIO *io, int encver) {
    if (encver > ES_ENC_VER) {
        return NULL;
    }
    ES *es = ES_CALLOC(1, sizeof(es));

    size_t hp_size, buckets_size;
    es->hp = (ES_HeavyPart *)RedisModule_LoadStringBuffer(io, &hp_size);
    assert(hp_size == sizeof(ES_HeavyPart));
    es->hp->bucket_num = RedisModule_LoadUnsigned(io);
    es->hp->buckets = (ES_HeavyPart_Bucket *)RedisModule_LoadStringBuffer(io, &buckets_size);
    assert(buckets_size == ((size_t)es->hp->bucket_num) * sizeof(ES_HeavyPart_Bucket));
    for (uint32_t i = 0; i < es->hp->bucket_num; ++i)
        for (uint32_t j = 0; j < COUNTER_PER_BUCKET; ++j) {
            size_t key_len;
            es->hp->buckets[i].key[j] = RedisModule_LoadStringBuffer(io, &key_len);
            if (key_len == 1) {
                RedisModule_Free(es->hp->buckets[i].key[j]);
                es->hp->buckets[i].key[j] = NULL;
            }
        }

    size_t lp_size, counters_size;
    es->lp = (ES_LightPart *)RedisModule_LoadStringBuffer(io, &lp_size);
    assert(lp_size == sizeof(ES_LightPart));
    es->lp->counters = (ES_LightPart_CounterType *)RedisModule_LoadStringBuffer(io, &counters_size);
    assert(counters_size == ((size_t)es->lp->counter_num) * sizeof(ES_LightPart_CounterType));

    return es;
}

static void ESFree(void *value) { ES_Destroy(value); }

static size_t ESMemUsage(const void *value) { return ES_MEM((ES *)value); }

static int Test_Cmd(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    RedisModule_ReplyWithLongLong(ctx, 123456);
    return REDISMODULE_OK;
}

int ESModule_onLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    // TODO: add option to set defaults from command line and in program
    RedisModule_Log(ctx, "notice", "ES Loaded");
    RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
                                 .rdb_load = ESRdbLoad,
                                 .rdb_save = ESRdbSave,
                                 .aof_rewrite = RMUtil_DefaultAofRewrite,
                                 .mem_usage = ESMemUsage,
                                 .free = ESFree};

    RedisModule_Log(ctx, "notice", "ES Create Type Start");
    ESType = RedisModule_CreateDataType(ctx, "ElSk-TYPE", ES_ENC_VER, &tm);
    if (ESType == NULL)
        return REDISMODULE_ERR;
    RedisModule_Log(ctx, "notice", "ES Create Type End");

    RedisModule_Log(ctx, "notice", "ES Create CMD Start");
    RMUtil_RegisterWriteDenyOOMCmd(ctx, "es.create", ES_Create_Cmd);
    RMUtil_RegisterWriteDenyOOMCmd(ctx, "es.insert", ES_Insert_Cmd);
    RMUtil_RegisterReadCmd(ctx, "es.query", ES_Query_Cmd);

    RMUtil_RegisterReadCmd(ctx, "test.test", Test_Cmd);
    RedisModule_Log(ctx, "notice", "ES Create CMD End");

    return REDISMODULE_OK;
}
