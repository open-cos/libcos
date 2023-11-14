//
// Created by david on 24/08/23.
//

#include "CosDataBuffer.h"

#include "common/memory-support.h"

#include <malloc.h>
#include <stdint.h>
#include <string.h>

struct CosDataBuffer {
    CosByte *bytes;
    size_t size;
    size_t capacity;
};

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
cos_data_buffer_free(CosDataBuffer *data_buffer)
{
    if (!data_buffer) {
        return;
    }

    free(data_buffer->bytes);
    free(data_buffer);
}

size_t
cos_data_buffer_get_size(const CosDataBuffer *data_buffer)
{
    if (!data_buffer) {
        return 0;
    }

    return data_buffer->size;
}

CosByte *
cos_data_buffer_get_bytes(const CosDataBuffer *data_buffer)
{
    if (!data_buffer) {
        return NULL;
    }

    return data_buffer->bytes;
}

size_t
cos_data_buffer_get_capacity(const CosDataBuffer *data_buffer)
{
    if (!data_buffer) {
        return 0;
    }

    return data_buffer->capacity;
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
    if (!data_buffer) {
        if (error) {
            *error = cos_error_alloc(COS_ERROR_INVALID_ARGUMENT,
                                     "Expected a non-NULL data buffer");
        }
        return false;
    }

    if (data_buffer->capacity == 0 ||
        data_buffer->size > (data_buffer->capacity - 1)) {
        const size_t new_capacity = data_buffer->size + 1;
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
    }

    data_buffer->bytes[data_buffer->size] = byte;
    data_buffer->size += 1;

    return true;
}

bool
cos_data_buffer_append(CosDataBuffer *data_buffer,
                       const CosByte *bytes,
                       size_t size)
{
    if (!data_buffer || !bytes || size == 0) {
        return false;
    }

    if ((data_buffer->size + size) > data_buffer->capacity) {
        const size_t new_capacity = data_buffer->size + size;
        CosByte * const new_bytes = realloc(data_buffer->bytes,
                                            new_capacity);
        if (!new_bytes) {
            return false;
        }

        data_buffer->bytes = new_bytes;
        data_buffer->capacity = new_capacity;
    }

    memcpy(data_buffer->bytes + data_buffer->size,
           bytes,
           size);
    data_buffer->size += size;

    return true;
}
