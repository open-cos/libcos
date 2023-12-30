//
// Created by david on 20/08/23.
//

#include "CosData.h"

#include <stdlib.h>
#include <string.h>

CosData *
cos_data_alloc(void)
{
    CosData * const data = malloc(sizeof(CosData));
    if (!data) {
        return NULL;
    }

    data->bytes = NULL;
    data->size = 0;

    return data;
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

size_t
cos_data_get_byte_range(const CosData *data,
                        size_t offset,
                        size_t length,
                        CosByte *buffer,
                        size_t buffer_size)
{
    if (!data) {
        return 0;
    }

    if (offset >= data->size) {
        return 0;
    }

    /// The number of bytes available.
    const size_t available_bytes = data->size - offset;
    /// The number of bytes that can be copied.
    const size_t bytes_to_copy = length < available_bytes ? length : available_bytes;
    /// The number of bytes actually copied.
    const size_t bytes_copied = bytes_to_copy < buffer_size ? bytes_to_copy : buffer_size;

    memcpy(buffer,
           data->bytes + offset,
           bytes_copied);

    return bytes_copied;
}
