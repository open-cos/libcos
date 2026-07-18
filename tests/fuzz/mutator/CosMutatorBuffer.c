/*
 * Copyright (c) 2026 OpenCOS.
 */

#include "CosMutatorBuffer.h"

#include "common/Assert.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

/**
 * Ensures the buffer can hold @p capacity bytes, growing geometrically.
 *
 * The capacity is independent of @c max_length: the cap bounds the contents,
 * this bounds the allocation. Growth is amortized so that the steady state of a
 * fuzzing run performs no allocation at all.
 */
static bool
cos_mut_buffer_reserve_(CosMutBuffer *buffer,
                        size_t capacity)
{
    COS_IMPL_PARAM_CHECK(buffer != NULL);

    if (capacity <= buffer->capacity) {
        return true;
    }

    size_t new_capacity = (buffer->capacity != 0) ? buffer->capacity : 256;
    while (new_capacity < capacity) {
        /* Guard the doubling itself against overflow. */
        if (new_capacity > (SIZE_MAX / 2)) {
            new_capacity = capacity;
            break;
        }
        new_capacity *= 2;
    }

    unsigned char * const data = realloc(buffer->data, new_capacity);
    if (!data) {
        return false;
    }

    buffer->data = data;
    buffer->capacity = new_capacity;

    return true;
}

void
cos_mut_buffer_init_(CosMutBuffer *buffer)
{
    COS_IMPL_PARAM_CHECK(buffer != NULL);

    buffer->data = NULL;
    buffer->length = 0;
    buffer->capacity = 0;
    buffer->max_length = 0;
}

void
cos_mut_buffer_free_(CosMutBuffer *buffer)
{
    COS_IMPL_PARAM_CHECK(buffer != NULL);

    free(buffer->data);

    cos_mut_buffer_init_(buffer);
}

void
cos_mut_buffer_set_max_(CosMutBuffer *buffer,
                        size_t max_length)
{
    COS_IMPL_PARAM_CHECK(buffer != NULL);

    buffer->max_length = max_length;

    if (buffer->length > max_length) {
        buffer->length = max_length;
    }
}

bool
cos_mut_buffer_assign_(CosMutBuffer *buffer,
                       const unsigned char * COS_Nullable data,
                       size_t size)
{
    COS_IMPL_PARAM_CHECK(buffer != NULL);

    if (size > buffer->max_length) {
        size = buffer->max_length;
    }

    if (size == 0) {
        buffer->length = 0;
        return true;
    }

    if (!cos_mut_buffer_reserve_(buffer, size)) {
        return false;
    }

    if (data) {
        memcpy(buffer->data, data, size);
    }
    buffer->length = size;

    return true;
}

bool
cos_mut_buffer_splice_(CosMutBuffer *buffer,
                       size_t offset,
                       size_t remove_size,
                       const unsigned char * COS_Nullable insert,
                       size_t insert_size)
{
    COS_IMPL_PARAM_CHECK(buffer != NULL);

    if (offset > buffer->length ||
        remove_size > (buffer->length - offset)) {
        return false;
    }

    const size_t tail_size = buffer->length - offset - remove_size;
    const size_t new_length = offset + insert_size + tail_size;

    if (new_length > buffer->max_length) {
        return false;
    }

    if (!cos_mut_buffer_reserve_(buffer, new_length)) {
        return false;
    }

    unsigned char * const data = buffer->data;

    /*
     * Move the tail before writing the insertion, so that a shrinking splice
     * does not overwrite bytes it has yet to move. memmove handles the overlap
     * in both directions.
     */
    if (tail_size > 0 && insert_size != remove_size) {
        memmove(data + offset + insert_size,
                data + offset + remove_size,
                tail_size);
    }

    if (insert_size > 0 && insert) {
        memcpy(data + offset, insert, insert_size);
    }

    buffer->length = new_length;

    return true;
}

void
cos_mut_buffer_truncate_(CosMutBuffer *buffer,
                         size_t length)
{
    COS_IMPL_PARAM_CHECK(buffer != NULL);

    if (length < buffer->length) {
        buffer->length = length;
    }
}

COS_ASSUME_NONNULL_END
