//#include <math.h>     ceil, log10f
//#include <strings.h>  strncasecmp
#include <assert.h>

#include "version.h"
#include "rmutil/util.h"

#include "cc.h"
#include "rm_cc.h"

#define INNER_ERROR(x)                                                                             \
    RedisModule_ReplyWithError(ctx, x);                                                            \
    return REDISMODULE_ERR;

RedisModuleType *CCType;

static int GetCCKey(RedisModuleCtx *ctx, RedisModuleString *keyName, CCSketch **cc, int mode) {
    // All using this function should call RedisModule_AutoMemory to prevent memory leak
    RedisModuleKey *key = RedisModule_OpenKey(ctx, keyName, mode);
    if (RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY) {
        RedisModule_CloseKey(key);
        INNER_ERROR("CC: key does not exist");
    } else if (RedisModule_ModuleTypeGetType(key) != CCType) {
        RedisModule_CloseKey(key);
        INNER_ERROR(REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    *cc = RedisModule_ModuleTypeGetValue(key);
    RedisModule_CloseKey(key);
    return REDISMODULE_OK;
}

static int createCC(RedisModuleCtx *ctx, RedisModuleString **argv, int argc, CCSketch **cc) {
    long long width;
    if (argc == 3) {
        if ((RedisModule_StringToLongLong(argv[2], &width) != REDISMODULE_OK) ||
            width < 1 || width > 100000) {
            INNER_ERROR("CC: invalid width size");
        }
    } else {
        width = 100000;
    }
    *cc = NewCCSketch(width, 2);
    return REDISMODULE_OK;
}

static int CC_Create_Cmd(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc != 2 && argc != 3) {
        return RedisModule_WrongArity(ctx);
    }

    RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    if (RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_EMPTY) {
        RedisModule_ReplyWithError(ctx, "CC: key already exists");
        goto final;
    }

    CCSketch *cc = NULL;
    if (createCC(ctx, argv, argc, &cc) != REDISMODULE_OK)
        goto final;

    if (RedisModule_ModuleTypeSetValue(key, CCType, cc) == REDISMODULE_ERR) {
        goto final;
    }

    RedisModule_ReplicateVerbatim(ctx);
    RedisModule_ReplyWithSimpleString(ctx, "OK");
final:
    RedisModule_CloseKey(key);
    return REDISMODULE_OK;
}

static int CC_Insert_Cmd(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc < 3)
        return RedisModule_WrongArity(ctx);

    CCSketch *cc;
    if (GetCCKey(ctx, argv[1], &cc, REDISMODULE_READ | REDISMODULE_WRITE) != REDISMODULE_OK) {
        return REDISMODULE_OK;
    }

    int itemCount = argc - 2;
    RedisModule_ReplyWithArray(ctx, itemCount);

    for (int i = 0; i < itemCount; ++i) {
        size_t itemlen;
        const char *item = RedisModule_StringPtrLen(argv[i + 2], &itemlen);
        CCS_IncrBy(cc, item, itemlen, 1);

        RedisModule_ReplyWithNull(ctx);
    }
    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}


static int CC_Query_Cmd(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc < 3)
        return RedisModule_WrongArity(ctx);

    CCSketch *cc;
    if (GetCCKey(ctx, argv[1], &cc, REDISMODULE_READ) != REDISMODULE_OK)
        return REDISMODULE_ERR;

    size_t itemlen;
    long long res;
    RedisModule_ReplyWithArray(ctx, argc - 2);
    for (int i = 2; i < argc; ++i) {
        const char *item = RedisModule_StringPtrLen(argv[i], &itemlen);
        res = CCS_Query(cc, item, itemlen);
        RedisModule_ReplyWithLongLong(ctx, res);
    }

    return REDISMODULE_OK;
}

/**************** Module functions *********************************/

static void CCRdbSave(RedisModuleIO *io, void *obj) {
    CCSketch *cc = obj;

    RedisModule_SaveUnsigned(io, cc->width);
    RedisModule_SaveUnsigned(io, cc->depth);
    /*RedisModule_SaveStringBuffer(io, (const char *)cc->HK,
                                 CC_d * (MAX_MEM+10) * BN * sizeof(node));*/
    for (size_t i=0; i<CC_d; i++){
	for (size_t j=0; j<MAX_MEM+10; j++){
            for (size_t k=0; k<BN; k++){
	    	RedisModule_SaveUnsigned(io, cc->HK[i][j][k].C);
		RedisModule_SaveUnsigned(io, cc->HK[i][j][k].FP);
	    }
	}
    }
}

static void *CCRdbLoad(RedisModuleIO *io, int encver) {
    if (encver > CC_ENC_VER) {
        return NULL;
    }
    CCSketch *cc = CCS_CALLOC(1, sizeof(CCSketch));

    cc->width = RedisModule_LoadUnsigned(io);
    cc->depth = RedisModule_LoadUnsigned(io);
    for (size_t i=0; i<CC_d; i++){
	for (size_t j=0; j<MAX_MEM+10; j++){
            for (size_t k=0; k<BN; k++){
	    	cc->HK[i][j][k].C = RedisModule_LoadUnsigned(io);
		cc->HK[i][j][k].FP = RedisModule_LoadUnsigned(io);
	    }
	}
    }
   
    return cc;
}

static void CCFree(void *value) { CCS_Destroy(value); }

static size_t CCMemUsage(const void *value) { 
    CCSketch *cc = (CCSketch *)value;
    return sizeof(cc);
}

static int Test_Cmd(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    RedisModule_ReplyWithLongLong(ctx, 123456);
    return REDISMODULE_OK;
}

int CCModule_onLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    // TODO: add option to set defaults from command line and in program
    RedisModule_Log(ctx, "notice", "CC Loaded");
    RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
                                 .rdb_load = CCRdbLoad,
                                 .rdb_save = CCRdbSave,
                                 .aof_rewrite = RMUtil_DefaultAofRewrite,
                                 .mem_usage = CCMemUsage,
                                 .free = CCFree};

    RedisModule_Log(ctx, "notice", "CC Create Type Start");
    CCType = RedisModule_CreateDataType(ctx, "CCSk-TYPE", CC_ENC_VER, &tm);
    if (CCType == NULL)
        return REDISMODULE_ERR;
    RedisModule_Log(ctx, "notice", "CC Create Type End");

    RedisModule_Log(ctx, "notice", "CC Create CMD Start");
    RMUtil_RegisterWriteDenyOOMCmd(ctx, "cc.create", CC_Create_Cmd);
    RMUtil_RegisterWriteDenyOOMCmd(ctx, "cc.insert", CC_Insert_Cmd);
    RMUtil_RegisterReadCmd(ctx, "cc.query", CC_Query_Cmd);

    RMUtil_RegisterReadCmd(ctx, "test.test", Test_Cmd);
    RedisModule_Log(ctx, "notice", "CC Create CMD End");

    return REDISMODULE_OK;
}
