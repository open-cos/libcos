/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COS_ARRAY_H
#define LIBCOS_COS_ARRAY_H

#include <libcos/common/CosDefines.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef struct CosArray CosArray;

typedef void (*CosArrayRetainItemCallback)(void *item);
typedef void (*CosArrayReleaseItemCallback)(void *item);
typedef bool (*CosArrayEqualItemsCallback)(const void *item1,
                                           const void *item2);

typedef struct CosArrayCallbacks {
    CosArrayRetainItemCallback COS_Nullable retain;
    CosArrayReleaseItemCallback COS_Nullable release;
    CosArrayEqualItemsCallback COS_Nullable equal;
} CosArrayCallbacks;

struct CosArray {
    void **items;
    size_t count;
    size_t capacity;

    CosArrayCallbacks callbacks;
};

CosArray * COS_Nullable
cos_array_alloc(CosArrayCallbacks callbacks)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_array_free(CosArray *array);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_ARRAY_H */
