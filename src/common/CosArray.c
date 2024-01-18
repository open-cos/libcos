/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/common/CosArray.h"

#include "common/Assert.h"
#include "common/CosUtils.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

struct CosArray {
    unsigned char *data COS_NONSTRING;
    size_t element_size;
    size_t count;
    size_t capacity;

    CosArrayCallbacks callbacks;
};

static size_t
cos_round_capacity_(size_t capacity)
{
    if (capacity < 4) {
        return 4;
    }
    // Round up to the next power of 2.
    const size_t next_capacity = 1UL << cos_flsl((long)capacity);
    if (next_capacity > LONG_MAX) {
        return LONG_MAX;
    }
    return next_capacity;
}

CosArray *
cos_array_alloc(size_t element_size,
                CosArrayCallbacks callbacks,
                size_t capacity_hint)
{
    COS_PARAMETER_ASSERT(element_size > 0);
    if (element_size == 0) {
        return NULL;
    }

    CosArray *array = NULL;
    unsigned char *data = NULL;

    array = malloc(sizeof(CosArray));
    if (!array) {
        goto failure;
    }

    const size_t capacity = cos_round_capacity_(capacity_hint);

    data = calloc(capacity, element_size);
    if (!data) {
        goto failure;
    }

    array->data = data;
    array->element_size = element_size;
    array->count = 0;
    array->capacity = capacity;

    array->callbacks = callbacks;

    return array;

failure:
    if (array) {
        free(array);
    }
    if (data) {
        free(data);
    }
    return NULL;
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
            void * const item = array->data + (i * array->element_size);
            release(item);
        }
    }

    free(array->data);
    free(array);
}

bool
cos_array_append(CosArray *array,
                 const void *item,
                 CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(array != NULL);
    COS_PARAMETER_ASSERT(item != NULL);
    if (!array || !item) {
        return false;
    }

    const size_t count = array->count;
    const size_t capacity = array->capacity;
    if (count + 1 >= capacity) {
        const size_t new_capacity = cos_round_capacity_(capacity * 2);
        unsigned char * const new_data = realloc(array->data,
                                                 new_capacity * array->element_size);
        if (!new_data) {
            return false;
        }

        array->data = new_data;
        array->capacity = new_capacity;
    }

    memmove(array->data + (count * array->element_size),
            item,
            array->element_size);
    array->count = count + 1;

    return true;
}

COS_ASSUME_NONNULL_END
