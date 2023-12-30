/*
 * Copyright (c) 2023 OpenCOS.
 */

#include <libcos/common/CosArray.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

CosArray *
cos_array_alloc(CosArrayCallbacks callbacks)
{
    CosArray * const array = malloc(sizeof(CosArray));
    if (!array) {
        return NULL;
    }

    array->items = NULL;
    array->count = 0;
    array->capacity = 0;

    array->callbacks = callbacks;

    return array;
}

void
cos_array_free(CosArray *array)
{
    if (!array) {
        return;
    }

    const CosArrayReleaseItemCallback release = array->callbacks.release;
    if (release) {
        const size_t count = array->count;
        for (size_t i = 0; i < count; i++) {
            release(array->items[i]);
        }
    }

    free(array->items);
    free(array);
}

COS_ASSUME_NONNULL_END
