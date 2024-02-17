/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/common/CosArray.h"

#include "common/Assert.h"
#include "common/CosUtils.h"

#include <libcos/common/CosError.h>

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

/**
 * @brief Rounds up the capacity to the next power of 2.
 *
 * @param capacity The capacity to be rounded.
 *
 * @return The rounded capacity.
 */
static size_t
cos_round_capacity_(size_t capacity)
{
    if (capacity < 4) {
        return 4;
    }
    // Round up to the next power of 2.
    return cos_next_pow2l(capacity + 1);
}

static void
cos_apply_func_(void *items,
                size_t count,
                size_t element_size,
                void (*func)(void *item));

static bool
cos_array_insert_items_(CosArray *array,
                        size_t index,
                        void *items,
                        size_t count,
                        CosError * COS_Nullable error);

static bool
cos_array_remove_items_(CosArray *array,
                        size_t index,
                        size_t count,
                        CosError * COS_Nullable error);

static bool
cos_array_resize_(CosArray *array,
                  size_t required_capacity);

#pragma mark - Public

CosArray *
cos_array_alloc(size_t element_size,
                const CosArrayCallbacks * COS_Nullable callbacks,
                size_t capacity_hint)
{
    COS_PARAMETER_ASSERT(element_size > 0);
    if (element_size == 0) {
        return NULL;
    }

    CosArray *array = NULL;
    unsigned char *data = NULL;

    array = calloc(1, sizeof(CosArray));
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

    if (callbacks) {
        array->callbacks = *callbacks;
    }

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

#pragma mark - Accessors

size_t
cos_array_get_count(const CosArray *array)
{
    COS_PARAMETER_ASSERT(array != NULL);
    if (!array) {
        return 0;
    }

    return array->count;
}

size_t
cos_array_get_capacity(const CosArray *array)
{
    COS_PARAMETER_ASSERT(array != NULL);
    if (!array) {
        return 0;
    }

    return array->capacity;
}

void *
cos_array_get_item(const CosArray *array,
                   size_t index,
                   CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(array != NULL);
    if (!array) {
        return NULL;
    }

    if (index >= array->count) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Index out of bounds"),
                            error);
        return NULL;
    }

    return (array->data + (index * array->element_size));
}

#pragma mark Insertion

bool
cos_array_insert_item(CosArray *array,
                      size_t index,
                      void *item,
                      CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(array != NULL);
    COS_PARAMETER_ASSERT(item != NULL);
    if (!array || !item) {
        return false;
    }

    return cos_array_insert_items(array, index, item, 1, error);
}

bool
cos_array_append_item(CosArray *array,
                      void *item,
                      CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(array != NULL);
    COS_PARAMETER_ASSERT(item != NULL);
    if (!array) {
        return false;
    }

    // Insert the item at the end of the array.
    return cos_array_insert_items_(array, array->count, item, 1, error);
}

bool
cos_array_insert_items(CosArray *array,
                       size_t index,
                       void *items,
                       size_t count,
                       CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(array != NULL);
    COS_PARAMETER_ASSERT(items != NULL);

    return cos_array_insert_items_(array, index, items, count, error);
}

bool
cos_array_append_items(CosArray *array,
                       void *items,
                       size_t count,
                       CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(array != NULL);
    COS_PARAMETER_ASSERT(items != NULL);
    if (!array) {
        return false;
    }

    // Insert the items at the end of the array.
    return cos_array_insert_items_(array, array->count, items, count, error);
}

#pragma mark Removal

bool
cos_array_remove_item(CosArray *array,
                      size_t index,
                      CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(array != NULL);

    return cos_array_remove_items_(array, index, 1, error);
}

bool
cos_array_remove_items(CosArray *array,
                       size_t index,
                       size_t count,
                       CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(array != NULL);

    return cos_array_remove_items_(array, index, count, error);
}

#pragma mark - Private

static void
cos_apply_func_(void *items,
                size_t count,
                size_t element_size,
                void (*func)(void *item))
{
    COS_PARAMETER_ASSERT(items != NULL);
    COS_PARAMETER_ASSERT(func != NULL);
    if (!items || !func) {
        return;
    }

    if (count == 0 || element_size == 0) {
        // Nothing to do.
        return;
    }

    unsigned char * const data = items;
    for (size_t i = 0; i < count; i++) {
        void * const item = data + (i * element_size);
        func(item);
    }
}

static bool
cos_array_insert_items_(CosArray *array,
                        size_t index,
                        void *items,
                        size_t count,
                        CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(array != NULL);
    COS_PARAMETER_ASSERT(items != NULL);
    if (!array || !items) {
        return false;
    }

    if (count == 0) {
        // Nothing to do.
        return true;
    }

    // Ensure that the index is within the bounds of the array.
    const size_t current_count = array->count;
    if (index > current_count) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Index out of bounds"),
                            error);
        return false;
    }

    // Ensure that the array has enough capacity to hold the new items.
    const size_t required_capacity = current_count + count;
    if (required_capacity > array->capacity && !cos_array_resize_(array, required_capacity)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to resize array"),
                            error);
        return false;
    }

    const size_t element_size = array->element_size;
    unsigned char * const data = array->data;

    // Retain the new items.
    CosArrayRetainItemCallback retain_callback = array->callbacks.retain;
    if (retain_callback) {
        cos_apply_func_(items, count, element_size, retain_callback);
    }

    // We only need to shift existing items if we're inserting in the middle of the array.
    if (index < current_count) {
        // Shift the items after the insertion point to the end of the insertion range.
        memmove(data + ((index + count) * element_size),
                data + (index * element_size),
                (current_count - index) * element_size);
    }

    // Copy the new items into the insertion range.
    memcpy(data + (index * element_size),
           items,
           count * element_size);
    array->count += count;

    return true;
}

static bool
cos_array_remove_items_(CosArray *array,
                        size_t index,
                        size_t count,
                        CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(array != NULL);
    if (!array) {
        return false;
    }

    if (count == 0) {
        // Nothing to do.
        return true;
    }

    // Ensure that the index is within the bounds of the array.
    const size_t current_count = array->count;
    if (index >= current_count) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Index out of bounds"),
                            error);
        return false;
    }

    const size_t element_size = array->element_size;
    unsigned char * const data = array->data;

    const size_t removal_byte_offset = index * element_size;
    const size_t removal_byte_count = count * element_size;

    void * const removal_start = data + removal_byte_offset;

    // Release the removed items.
    const CosArrayReleaseItemCallback release_callback = array->callbacks.release;
    if (release_callback) {
        void * const items = removal_start;
        cos_apply_func_(items, count, element_size, release_callback);
    }

    const size_t trailing_item_count = current_count - (index + count);

    // We only need to shift existing items if we're removing from the middle of the array.
    if (trailing_item_count > 0) {
        const void * const trailing_items = (data + removal_byte_offset) + removal_byte_count;
        // Shift the items after the removal point to the start of the removal range.
        memmove(removal_start,
                trailing_items,
                trailing_item_count * element_size);
    }

    array->count -= count;

    return true;
}

static bool
cos_array_resize_(CosArray *array,
                  size_t required_capacity)
{
    COS_PARAMETER_ASSERT(array != NULL);
    if (!array) {
        return false;
    }

    if (array->capacity >= required_capacity) {
        // No need to resize.
        return true;
    }

    const size_t new_capacity = cos_round_capacity_(required_capacity);

    unsigned char * const new_data = realloc(array->data,
                                             new_capacity * array->element_size);
    if (!new_data) {
        return false;
    }

    array->data = new_data;
    array->capacity = new_capacity;

    return true;
}

COS_ASSUME_NONNULL_END
