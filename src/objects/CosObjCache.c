/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "objects/CosObjCache.h"

#include "common/Assert.h"

#include <libcos/common/CosDict.h>
#include <libcos/common/CosError.h>
#include <libcos/objects/CosObjNode.h>

#include <stdint.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosObjCache {
    CosDict *dict;
};

// MARK: - Dict callbacks

static bool
cos_obj_cache_keys_equal_(void *key1,
                          void *key2)
{
    return key1 == key2;
}

static void
cos_obj_cache_value_retain_(void *value)
{
    cos_obj_node_retain((CosObjNode *)value);
}

static void
cos_obj_cache_value_release_(void *value)
{
    cos_obj_node_release((CosObjNode *)value);
}

static const CosDictKeyCallbacks cos_obj_cache_key_callbacks_ = {
    .hash = NULL,
    .retain = NULL,
    .release = NULL,
    .equal = &cos_obj_cache_keys_equal_,
};

static const CosDictValueCallbacks cos_obj_cache_value_callbacks_ = {
    .retain = &cos_obj_cache_value_retain_,
    .release = &cos_obj_cache_value_release_,
    .equal = NULL,
};

// MARK: - Lifecycle

CosObjCache *
cos_obj_cache_create(size_t capacity_hint)
{
    CosObjCache *cache = NULL;
    CosDict *dict = NULL;

    cache = calloc(1, sizeof(CosObjCache));
    if (!cache) {
        goto failure;
    }

    dict = cos_dict_create(&cos_obj_cache_key_callbacks_,
                           &cos_obj_cache_value_callbacks_,
                           capacity_hint);
    if (!dict) {
        goto failure;
    }
    cache->dict = COS_nonnull_cast(dict);

    return cache;

failure:
    if (cache) {
        free(cache);
    }
    if (dict) {
        cos_dict_destroy(dict);
    }
    return NULL;
}

void
cos_obj_cache_destroy(CosObjCache *cache)
{
    if (!cache) {
        return;
    }

    cos_dict_destroy(cache->dict);
    free(cache);
}

// MARK: - Operations

CosObjNode *
cos_obj_cache_get(CosObjCache *cache,
                  unsigned int obj_number)
{
    COS_API_PARAM_CHECK(cache != NULL);
    if (COS_UNLIKELY(!cache)) {
        return NULL;
    }

    void *value = NULL;
    if (cos_dict_get(cache->dict,
                     (void *)(uintptr_t)obj_number,
                     &value,
                     NULL)) {
        return (CosObjNode *)value;
    }

    return NULL;
}

bool
cos_obj_cache_insert(CosObjCache *cache,
                     unsigned int obj_number,
                     CosObjNode *obj)
{
    COS_API_PARAM_CHECK(cache != NULL);
    COS_API_PARAM_CHECK(obj != NULL);
    if (COS_UNLIKELY(!cache || !obj)) {
        return false;
    }

    return cos_dict_set(cache->dict,
                        (void *)(uintptr_t)obj_number,
                        obj,
                        NULL);
}

COS_ASSUME_NONNULL_END
