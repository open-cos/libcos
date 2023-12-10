//
// Created by david on 24/08/23.
//

#include "CosDataBuffer.h"

#include "common/memory-support.h"

#include <malloc.h>
#include <stdint.h>
#include <string.h>

static bool
cos_data_buffer_resize_(CosDataBuffer *data_buffer,
                        size_t required_capacity,
                        CosError **error);

CosDataBuffer *
cos_data_buffer_alloc(void)
{
    CosDataBuffer * const data_buffer = malloc(sizeof(CosDataBuffer));
    if (!data_buffer) {
        return NULL;
    }

    data_buffer->bytes = NULL;
    data_buffer->size = 0;
    data_buffer->capacity = 0;

    return data_buffer;
}

void
cos_data_buffer_init(CosDataBuffer *data_buffer)
{
    if (!data_buffer) {
        return;
    }

    data_buffer->bytes = NULL;
    data_buffer->size = 0;
    data_buffer->capacity = 0;
}

void
cos_data_buffer_destroy(CosDataBuffer *data_buffer)
{
    if (!data_buffer) {
        return;
    }

    free(data_buffer->bytes);
    data_buffer->bytes = NULL;
    data_buffer->size = 0;
    data_buffer->capacity = 0;
}

bool
cos_data_buffer_reserve(CosDataBuffer *data_buffer,
                        size_t capacity,
                        CosError **error)
{
    if (!data_buffer) {
        return false;
    }

    if (data_buffer->capacity >= capacity) {
        return true;
    }

    CosByte * const new_bytes = realloc(data_buffer->bytes,
                                        capacity * sizeof(CosByte));
    if (!new_bytes) {
        if (error) {
            *error = cos_error_alloc(COS_ERROR_MEMORY,
                                     "Failed to allocate memory for data buffer");
        }
        return false;
    }

    data_buffer->bytes = new_bytes;
    data_buffer->capacity = capacity;

    return true;
}

void
cos_data_buffer_free(CosDataBuffer *data_buffer)
{
    if (!data_buffer) {
        return;
    }

    free(data_buffer->bytes);
    free(data_buffer);
}

bool
cos_data_buffer_reset(CosDataBuffer *data_buffer)
{
    if (!data_buffer) {
        return false;
    }

    data_buffer->size = 0;

    return true;
}

bool
cos_data_buffer_push_back(CosDataBuffer *data_buffer,
                          CosByte byte,
                          CosError **error)
{
    return cos_data_buffer_append(data_buffer,
                                  &byte,
                                  1,
                                  error);
}

bool
cos_data_buffer_append(CosDataBuffer *data_buffer,
                       const CosByte *bytes,
                       size_t count,
                       CosError **error)
{
    if (!data_buffer || !bytes || count == 0) {
        return false;
    }

    const size_t required_capacity = data_buffer->size + count;
    if (required_capacity > data_buffer->capacity) {
        // TODO: Use a better growth strategy.
        if (!cos_data_buffer_resize_(data_buffer,
                                     required_capacity,
                                     error)) {
            return false;
        }
    }

    memcpy(data_buffer->bytes + data_buffer->size,
           bytes,
           count);
    data_buffer->size += count;

    return true;
}

static bool
cos_data_buffer_resize_(CosDataBuffer *data_buffer,
                        size_t required_capacity,
                        CosError **error)
{
    if (!data_buffer) {
        return false;
    }

    if (data_buffer->capacity >= required_capacity) {
        return true;
    }

    const size_t new_capacity = required_capacity;

    CosByte * const new_bytes = realloc(data_buffer->bytes,
                                        new_capacity);
    if (!new_bytes) {
        if (error) {
            *error = cos_error_alloc(COS_ERROR_MEMORY,
                                     "Failed to allocate memory for data buffer");
        }
        return false;
    }

    data_buffer->bytes = new_bytes;
    data_buffer->capacity = new_capacity;

    return true;
}
