/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/common/CosData.h"

#include "common/Assert.h"

#include <libcos/common/CosError.h>

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

static bool
cos_data_resize_(CosData *data,
                 size_t required_capacity,
                 CosError * COS_Nullable error);

// MARK: - Public

CosData * COS_Nullable
cos_data_alloc(size_t capacity_hint)
{
    CosData *data = NULL;
    unsigned char *bytes = NULL;

    data = malloc(sizeof(CosData));
    if (!data) {
        goto failure;
    }

    bytes = malloc(capacity_hint);
    if (!bytes) {
        goto failure;
    }

    data->bytes = bytes;
    data->size = 0;
    data->capacity = capacity_hint;

    return data;

failure:
    if (data) {
        free(data);
    }
    if (bytes) {
        free(bytes);
    }
    return NULL;
}

void
cos_data_free(CosData *data)
{
    if (!data) {
        return;
    }

    free(data->bytes);
    free(data);
}

CosData *
cos_data_copy(const CosData *source,
              CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(source != NULL);
    if (!source) {
        return NULL;
    }

    CosData * const copy = cos_data_alloc(source->size);
    if (!copy) {
        goto failure;
    }

    // Ensure that the copy has enough capacity to hold the data.
    if (!cos_data_reserve(copy, source->size, error)) {
        goto failure;
    }

    // Use memcpy here because we know the source and destination do not overlap.
    memcpy(copy->bytes,
           source->bytes,
           source->size);
    copy->size = source->size;

    return copy;

failure:
    if (copy) {
        cos_data_free(copy);
    }
    return NULL;
}

CosDataRef
cos_data_get_range(const CosData *data,
                   size_t offset,
                   size_t length,
                   CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(data != NULL);
    if (!data || length == 0) {
        return cos_data_ref_make(NULL, 0);
    }

    if (offset >= data->size) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_OUT_OF_RANGE,
                                           "Offset is out of range"),
                            error);
        return cos_data_ref_make(NULL, 0);
    }

    const size_t available_length = data->size - offset;
    const size_t reference_length = (length < available_length) ? length : available_length;

    return cos_data_ref_make(data->bytes + offset, reference_length);
}

bool
cos_data_reserve(CosData *data,
                 size_t capacity,
                 CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(data != NULL);
    if (!data) {
        return false;
    }

    return cos_data_resize_(data, capacity, error);
}

bool
cos_data_reset(CosData *data)
{
    COS_PARAMETER_ASSERT(data != NULL);
    if (!data) {
        return false;
    }

    data->size = 0;

    return true;
}

bool
cos_data_append(CosData *data,
                const unsigned char *bytes,
                size_t count,
                CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(data != NULL);
    if (!data) {
        return false;
    }

    if (count == 0) {
        return true;
    }

    const size_t required_capacity = data->size + count;
    if (required_capacity > data->capacity &&
        !cos_data_resize_(data,
                          required_capacity,
                          error)) {
        return false;
    }

    // Use memmove instead of memcpy in case the source and destination overlap.
    memmove(data->bytes + data->size,
            bytes,
            count);
    data->size += count;

    return true;
}

bool
cos_data_push_back(CosData *data,
                   unsigned char byte,
                   CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(data != NULL);
    if (!data) {
        return false;
    }

    return cos_data_append(data, &byte, 1, error);
}

// MARK: Data reference

CosDataRef
cos_data_get_ref(const CosData *data)
{
    COS_PARAMETER_ASSERT(data != NULL);
    if (!data) {
        return (CosDataRef){0};
    }

    return cos_data_ref_make(data->bytes, data->size);
}

// MARK: - Private

static bool
cos_data_resize_(CosData *data,
                 size_t required_capacity,
                 CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(data != NULL);
    if (!data) {
        return false;
    }

    if (data->capacity >= required_capacity) {
        return true;
    }

    const size_t new_capacity = required_capacity;

    unsigned char * const new_bytes = realloc(data->bytes,
                                              new_capacity);
    if (!new_bytes) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to allocate memory for data buffer"),
                            error);
        return false;
    }

    data->bytes = new_bytes;
    data->capacity = new_capacity;

    return true;
}

COS_ASSUME_NONNULL_END
