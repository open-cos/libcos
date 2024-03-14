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
    CosDictHashCallback COS_Nullable hash;
    CosDictRetainCallback COS_Nullable retain;
    CosDictReleaseCallback COS_Nullable release;
    CosDictEqualValuesCallback COS_Nullable equal;
} CosDictKeyCallbacks;

typedef struct CosDictValueCallbacks {
    CosDictRetainCallback COS_Nullable retain;
    CosDictReleaseCallback COS_Nullable release;
    CosDictEqualValuesCallback COS_Nullable equal;
} CosDictValueCallbacks;

extern const CosDictKeyCallbacks cos_dict_obj_key_callbacks;
extern const CosDictValueCallbacks cos_dict_obj_value_callbacks;

CosDict * COS_Nullable
cos_dict_alloc(const CosDictKeyCallbacks * COS_Nullable key_callbacks,
               const CosDictValueCallbacks * COS_Nullable value_callbacks,
               size_t capacity_hint)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_dict_free(CosDict *dict);

const CosDictKeyCallbacks * COS_Nullable
cos_dict_get_key_callbacks(const CosDict *dict);

const CosDictValueCallbacks * COS_Nullable
cos_dict_get_value_callbacks(const CosDict *dict);

size_t
cos_dict_get_count(const CosDict *dict);

bool
cos_dict_set(CosDict *dict,
             void *key,
             void *value,
             CosError * COS_Nullable error);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_DICT_H */
