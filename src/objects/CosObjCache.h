/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_COS_OBJ_CACHE_H
#define LIBCOS_COS_OBJ_CACHE_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef struct CosObjCache CosObjCache;

/**
 * @brief Destroys the object cache, releasing all cached objects.
 *
 * @param cache The cache to destroy.
 */
void
cos_obj_cache_destroy(CosObjCache *cache)
    COS_DEALLOCATOR_FUNC;

/**
 * @brief Creates a new object cache.
 *
 * @param capacity_hint An estimate of how many objects will be cached.
 *
 * @return A new object cache, or NULL on allocation failure.
 */
CosObjCache * COS_Nullable
cos_obj_cache_create(size_t capacity_hint)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_obj_cache_destroy);

/**
 * @brief Looks up a cached object by object number.
 *
 * The returned pointer is not retained -- the caller does not own a new
 * reference. To keep the object beyond the cache's lifetime, the caller
 * must retain it explicitly.
 *
 * @param cache The object cache.
 * @param obj_number The object number to look up.
 *
 * @return The cached object, or NULL if not found.
 */
CosObjNode * COS_Nullable
cos_obj_cache_get(CosObjCache *cache,
                  unsigned int obj_number);

/**
 * @brief Inserts an object into the cache.
 *
 * The cache retains the object (increments its reference count).
 * If an object with the same object number is already cached, it is
 * replaced and released.
 *
 * @param cache The object cache.
 * @param obj_number The object number.
 * @param obj The object to cache.
 *
 * @return true on success, false on allocation failure.
 */
bool
cos_obj_cache_insert(CosObjCache *cache,
                     unsigned int obj_number,
                     CosObjNode *obj);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_OBJ_CACHE_H */
