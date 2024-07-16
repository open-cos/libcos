/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_DICT_H
#define LIBCOS_COMMON_COS_DICT_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef size_t (*CosDictHashCallback)(void *value);
typedef void (*CosDictRetainCallback)(void *value);
typedef void (*CosDictReleaseCallback)(void *value);
typedef bool (*CosDictEqualValuesCallback)(void *value1,
                                           void *value2);

typedef struct CosDictKeyCallbacks {
    CosDictHashCallback hash;
    CosDictRetainCallback COS_Nullable retain;
    CosDictReleaseCallback COS_Nullable release;
    CosDictEqualValuesCallback equal;
} CosDictKeyCallbacks;

typedef struct CosDictValueCallbacks {
    CosDictRetainCallback COS_Nullable retain;
    CosDictReleaseCallback COS_Nullable release;
    CosDictEqualValuesCallback COS_Nullable equal;
} CosDictValueCallbacks;

void
cos_dict_destroy(CosDict *dict)
    COS_DEALLOCATOR_FUNC;

CosDict * COS_Nullable
cos_dict_create(const CosDictKeyCallbacks *key_callbacks,
                const CosDictValueCallbacks *value_callbacks,
                size_t capacity_hint)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_dict_destroy);

size_t
cos_dict_get_count(const CosDict *dict);

bool
cos_dict_get(CosDict *dict,
             void *key,
             void * COS_Nullable * COS_Nonnull out_value,
             CosError * COS_Nullable out_error);

bool
cos_dict_set(CosDict *dict,
             void *key,
             void *value,
             CosError * COS_Nullable error);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_DICT_H */
