#ifndef ES_MODULE_H
#define ES_MODULE_H

#include "redismodule.h"

#define ES_ENC_VER 0
#define REDIS_MODULE_TARGET

int ESModule_onLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

#endif