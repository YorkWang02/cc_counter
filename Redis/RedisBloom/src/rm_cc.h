#ifndef CC_MODULE_H
#define CC_MODULE_H

#include "redismodule.h"

#define CC_ENC_VER 0
#define REDIS_MODULE_TARGET

int CCModule_onLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

#endif
