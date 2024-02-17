/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "CosBufferedInputStream.h"

#include "common/Assert.h"

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

struct CosBufferedInputStream {
    CosInputStream base;

    CosInputStream * COS_Nullable base_stream;

    size_t buffer_size;
    size_t buffer_capacity;
    size_t buffer_pos;
    unsigned char *buffer;
};

static size_t
cos_buffered_input_stream_read_func_(CosInputStream *stream,
                                     void *output_buffer,
                                     size_t size,
                                     void * COS_Nullable user_data);

static bool
cos_buffered_input_stream_error_func_(CosInputStream *stream,
                                      void * COS_Nullable user_data);

static bool
cos_buffered_input_stream_eof_func_(CosInputStream *stream,
                                    void * COS_Nullable user_data);

static void
cos_buffered_input_stream_close_func_(CosInputStream *stream,
                                      void * COS_Nullable user_data);

//

static size_t
cos_buffered_input_stream_available_buffered_bytes_(const CosBufferedInputStream *self);

static size_t
cos_buffered_input_stream_read_from_buffer_(CosBufferedInputStream *self,
                                            unsigned char *output_buffer,
                                            size_t size)
    COS_ATTR_ACCESS_WRITE_ONLY_SIZE(2, 3);

/**
 * @brief Fills the buffer with data from the base stream.
 *
 * @param self The buffered input stream.
 * @return The number of bytes read into the buffer.
 */
static size_t
cos_buffered_input_stream_fill_buffer_(CosBufferedInputStream *self);

CosBufferedInputStream *
cos_buffered_input_stream_alloc(CosInputStream *base_stream,
                                size_t buffer_size)
{
    COS_PARAMETER_ASSERT(base_stream != NULL);
    if (!base_stream) {
        return NULL;
    }

    CosBufferedInputStream * const buffered_stream = malloc(sizeof(CosBufferedInputStream));
    if (!buffered_stream) {
        return NULL;
    }

    cos_buffered_input_stream_init(buffered_stream,
                                   base_stream,
                                   buffer_size);

    return buffered_stream;
}

void
cos_buffered_input_stream_init(CosBufferedInputStream *self,
                               CosInputStream *base_stream,
                               size_t buffer_size)
{
    COS_PARAMETER_ASSERT(self != NULL);
    COS_PARAMETER_ASSERT(base_stream != NULL);
    if (!self || !base_stream) {
        return;
    }

    cos_input_stream_init(&(self->base),
                          (CosInputStreamFunctions){
                              .read_func = cos_buffered_input_stream_read_func_,
                              .error_func = cos_buffered_input_stream_error_func_,
                              .eof_func = cos_buffered_input_stream_eof_func_,
                              .close_func = cos_buffered_input_stream_close_func_},
                          NULL);

    self->base_stream = base_stream;

    self->buffer_size = buffer_size;
    self->buffer_capacity = buffer_size;
    self->buffer_pos = 0;
    self->buffer = malloc(buffer_size);
}

static size_t
cos_buffered_input_stream_read_func_(CosInputStream *stream,
                                     void *output_buffer,
                                     size_t size,
                                     COS_ATTR_UNUSED void * COS_Nullable user_data)
{
    CosBufferedInputStream * const self = (CosBufferedInputStream *)stream;
    COS_PARAMETER_ASSERT(self != NULL);
    if (!self) {
        return 0;
    }

    /// The number of bytes read so far.
    size_t bytes_read = 0;
    /// The number of bytes left to read.
    size_t bytes_to_read = size;

    do {
        if (cos_buffered_input_stream_available_buffered_bytes_(self) > 0) {
            // Read from the buffer first.
            const size_t bytes_read_from_buffer = cos_buffered_input_stream_read_from_buffer_(self,
                                                                                              output_buffer,
                                                                                              bytes_to_read);
            COS_ASSERT(bytes_read_from_buffer <= bytes_to_read, "Read more bytes than requested.");
            bytes_read += bytes_read_from_buffer;
            bytes_to_read -= bytes_read_from_buffer;
        }

        // (Re)fill the buffer if it is empty.
        if (bytes_to_read > 0 && cos_buffered_input_stream_available_buffered_bytes_(self) == 0) {
            const size_t filled_bytes_count = cos_buffered_input_stream_fill_buffer_(self);
            if (filled_bytes_count == 0) {
                // Failed to fill the buffer.
                break;
            }
        }
    } while (bytes_to_read > 0 && cos_buffered_input_stream_available_buffered_bytes_(self) > 0);

    return bytes_read;
}

static bool
cos_buffered_input_stream_error_func_(CosInputStream *stream,
                                      COS_ATTR_UNUSED void * COS_Nullable user_data)
{
    CosBufferedInputStream * const self = (CosBufferedInputStream *)stream;
    COS_PARAMETER_ASSERT(self != NULL);
    if (!self) {
        return false;
    }

    return cos_input_stream_get_error(self->base_stream);
}

static bool
cos_buffered_input_stream_eof_func_(CosInputStream *stream,
                                    COS_ATTR_UNUSED void * COS_Nullable user_data)
{
    CosBufferedInputStream * const self = (CosBufferedInputStream *)stream;
    COS_PARAMETER_ASSERT(self != NULL);
    if (!self) {
        return true;
    }

    if (cos_buffered_input_stream_available_buffered_bytes_(self) > 0) {
        // There are still bytes left in the buffer.
        return false;
    }

    return cos_input_stream_is_eof(self->base_stream);
}

static void
cos_buffered_input_stream_close_func_(CosInputStream *stream,
                                      COS_ATTR_UNUSED void * COS_Nullable user_data)
{
    CosBufferedInputStream * const self = (CosBufferedInputStream *)stream;
    COS_PARAMETER_ASSERT(self != NULL);
    if (!self) {
        return;
    }

    cos_input_stream_close(self->base_stream);

    free(self->buffer);
}

//

static size_t
cos_buffered_input_stream_available_buffered_bytes_(const CosBufferedInputStream *self)
{
    COS_PARAMETER_ASSERT(self != NULL);
    if (!self) {
        return 0;
    }

    return self->buffer_size - self->buffer_pos;
}

static size_t
cos_buffered_input_stream_read_from_buffer_(CosBufferedInputStream *self,
                                            unsigned char *output_buffer,
                                            size_t size)
{
    COS_PARAMETER_ASSERT(self != NULL);
    COS_PARAMETER_ASSERT(output_buffer != NULL);
    if (!self || !output_buffer) {
        return 0;
    }

    size_t bytes_read = 0;
    size_t bytes_to_read = size;

    const size_t available_buffered_bytes = cos_buffered_input_stream_available_buffered_bytes_(self);
    if (bytes_to_read > available_buffered_bytes) {
        bytes_to_read = available_buffered_bytes;
    }

    // Skip reading zero bytes.
    if (bytes_to_read == 0) {
        return 0;
    }

    // Copy the bytes from the internal buffer over to the output buffer.
    memcpy(output_buffer,
           self->buffer + self->buffer_pos,
           bytes_to_read);

    bytes_read += bytes_to_read;

    // Advance the buffer position.
    self->buffer_pos += bytes_read;

    return bytes_read;
}

static size_t
cos_buffered_input_stream_fill_buffer_(CosBufferedInputStream *self)
{
    COS_PARAMETER_ASSERT(self != NULL);
    if (!self) {
        return 0;
    }

    self->buffer_size = cos_input_stream_read(self->base_stream,
                                              self->buffer,
                                              self->buffer_capacity);
    self->buffer_pos = 0;

    return self->buffer_size;
}

COS_ASSUME_NONNULL_END
