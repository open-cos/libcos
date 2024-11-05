/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/io/CosStream.h"

#include "common/Assert.h"

#include "libcos/common/CosError.h"
#include "libcos/common/CosMacros.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

#define COS_STREAM_BUFFER_SIZE 1024

typedef enum CosStreamBufferMode {
    CosStreamBufferMode_Read = 0,
    CosStreamBufferMode_Write = 1,
} CosStreamBufferMode;

/**
 * @brief A stream container.
 *
 * A stream is an abstraction over a sequence of data that is accessed via a set of client-provided
 * functions.
 */
struct CosStream {
    void *context;
    CosStreamFunctions functions;

    /**
     * @brief The buffered data.
     *
     * The buffer stores data read from or written to the stream. The buffer is used to reduce the
     * number of calls to the stream's read or write functions.
     */
    unsigned char *buffer;

    /**
     * @brief The size of the stream's buffer.
     */
    size_t buffer_size;

    /**
     * @brief The current position in the buffer.
     *
     * @invariant buffer_position <= buffer_length
     */
    size_t buffer_position;

    /**
     * @brief The length of valid data in the buffer.
     */
    size_t buffer_length;

    /**
     * @brief The current mode of the buffer.
     */
    CosStreamBufferMode buffer_mode;
};

// Static function prototypes.

static size_t
cos_stream_fill_read_buffer_(CosStream *stream,
                             size_t count,
                             CosError * COS_Nullable out_error);

static size_t
cos_stream_read_from_buffer_(CosStream *stream,
                             void *output_buffer,
                             size_t count);

CosStream *
cos_stream_create(const CosStreamFunctions *functions,
                  void *context)
{
    COS_PARAMETER_ASSERT(functions != NULL);
    if (COS_UNLIKELY(!functions)) {
        return NULL;
    }

    CosStream *stream = NULL;
    unsigned char *buffer = NULL;

    // Ensure that the stream functions are valid.
    if (!functions->eof_func ||
        !functions->close_func) {
        return NULL;
    }

    stream = calloc(1, sizeof(CosStream));
    if (COS_UNLIKELY(!stream)) {
        goto failure;
    }

    const size_t buffer_size = COS_STREAM_BUFFER_SIZE;

    buffer = malloc(buffer_size);
    if (COS_UNLIKELY(!buffer)) {
        goto failure;
    }

    stream->functions = *functions;
    stream->context = context;

    stream->buffer = buffer;
    stream->buffer_size = buffer_size;

    stream->buffer_position = 0;
    stream->buffer_length = 0;

    stream->buffer_mode = CosStreamBufferMode_Read;

    return stream;

failure:
    if (stream) {
        free(stream);
    }
    if (buffer) {
        free(buffer);
    }
    return NULL;
}

void
cos_stream_close(CosStream *stream)
{
    COS_PARAMETER_ASSERT(cos_stream_is_valid(stream));
    if (COS_UNLIKELY(!cos_stream_is_valid(stream))) {
        return;
    }

    if (stream->functions.close_func) {
        stream->functions.close_func(stream->context);
    }

    free(stream->buffer);

    free(stream);
}

bool
cos_stream_is_valid(const CosStream *stream)
{
    return (stream != NULL);
}

bool
cos_stream_can_read(const CosStream *stream)
{
    COS_API_PARAM_CHECK(stream != NULL);
    if (COS_UNLIKELY(!stream)) {
        return false;
    }

    return (stream->functions.read_func != NULL);
}

size_t
cos_stream_read(CosStream *stream,
                void *buffer,
                size_t count,
                CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(cos_stream_is_valid(stream));
    COS_PARAMETER_ASSERT(buffer != NULL);
    if (COS_UNLIKELY(!cos_stream_is_valid(stream) || !buffer)) {
        return 0;
    }

    if (!stream->functions.read_func) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Stream does not support reading"),
                            out_error);
        return 0;
    }

    // Skip doing any work if we don't need to read any data.
    if (COS_UNLIKELY(count == 0)) {
        return 0;
    }

    size_t total_bytes_read = 0;
    size_t remaining_count = count;

    while (total_bytes_read < count) {
        // If there is data in the buffer, read from it first.
        const size_t read_count = cos_stream_read_from_buffer_(stream,
                                                               ((unsigned char *)buffer) + total_bytes_read,
                                                               remaining_count);
        total_bytes_read += read_count;
        remaining_count -= read_count;

        // If we have read all the data we need, return.
        if (remaining_count == 0) {
            break;
        }

        // Re-fill the buffer with more data.
        const size_t filled_count = cos_stream_fill_read_buffer_(stream,
                                                                 remaining_count,
                                                                 out_error);
        if (filled_count == 0) {
            break;
        }
    }

    return total_bytes_read;
}

bool
cos_stream_can_write(const CosStream *stream)
{
    COS_API_PARAM_CHECK(stream != NULL);
    if (COS_UNLIKELY(!stream)) {
        return false;
    }

    return (stream->functions.write_func != NULL);
}

size_t
cos_stream_write(CosStream *stream,
                 const void *buffer,
                 size_t count,
                 CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(cos_stream_is_valid(stream));
    COS_PARAMETER_ASSERT(buffer != NULL);
    if (COS_UNLIKELY(!cos_stream_is_valid(stream) || !buffer)) {
        return 0;
    }

    if (!stream->functions.write_func) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Stream does not support writing"),
                            out_error);
        return 0;
    }

    return stream->functions.write_func(stream->context,
                                        buffer,
                                        count,
                                        out_error);
}

bool
cos_stream_can_seek(const CosStream *stream)
{
    COS_API_PARAM_CHECK(stream != NULL);
    if (COS_UNLIKELY(!stream)) {
        return false;
    }

    return (stream->functions.seek_func != NULL);
}

bool
cos_stream_seek(CosStream *stream,
                CosStreamOffset offset,
                CosStreamOffsetWhence whence,
                CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(cos_stream_is_valid(stream));
    if (COS_UNLIKELY(!cos_stream_is_valid(stream))) {
        return false;
    }

    if (!stream->functions.seek_func) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Stream does not support seeking"),
                            out_error);
        return false;
    }



    return stream->functions.seek_func(stream->context,
                                       offset,
                                       whence,
                                       out_error);
}

CosStreamOffset
cos_stream_get_position(CosStream *stream,
                        CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(cos_stream_is_valid(stream));
    if (COS_UNLIKELY(!cos_stream_is_valid(stream))) {
        return -1;
    }

    if (!stream->functions.tell_func) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Stream does not support telling"),
                            out_error);
        return -1;
    }

    return stream->functions.tell_func(stream->context,
                                       out_error);
}

bool
cos_stream_is_at_end(CosStream *stream,
                     CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(stream != NULL);
    if (COS_UNLIKELY(!stream)) {
        return true;
    }

    if (!stream->functions.eof_func) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Stream does not support checking for end"),
                            out_error);
        return true;
    }

    return stream->functions.eof_func(stream->context);
}

// Static functions.

static size_t
cos_stream_fill_read_buffer_(CosStream *stream,
                             size_t count,
                             CosError * COS_Nullable out_error)
{
    COS_PARAM_ASSERT_INTERNAL(stream != NULL);

    // Check if we have enough data in the buffer.
    if (stream->buffer_position + count <= stream->buffer_length) {
        return count;
    }

    // If we don't have enough data in the buffer, we need to read more.
    const size_t remaining_count = count - (stream->buffer_length - stream->buffer_position);

    // Shift the remaining data to the beginning of the buffer.
    memmove(stream->buffer,
            stream->buffer + stream->buffer_position,
            stream->buffer_length - stream->buffer_position);

    stream->buffer_position = 0;

    const size_t read_count = stream->functions.read_func(stream->context,
                                                          stream->buffer + stream->buffer_length - stream->buffer_position,
                                                          remaining_count,
                                                          out_error);

    stream->buffer_length += read_count;

    return read_count;
}

static size_t
cos_stream_read_from_buffer_(CosStream *stream,
                             void *output_buffer,
                             size_t count)
{
    COS_PARAM_ASSERT_INTERNAL(stream != NULL);
    COS_PARAM_ASSERT_INTERNAL(output_buffer != NULL);

    // Check that we have data available in the buffer.
    if (stream->buffer_position >= stream->buffer_length) {
        return 0;
    }

    const size_t buffered_byte_count = stream->buffer_length - stream->buffer_position;

    const size_t read_byte_count = COS_MIN(count, buffered_byte_count);

    memcpy(output_buffer,
           stream->buffer + stream->buffer_position,
           read_byte_count);

    // Update the buffer position.
    stream->buffer_position += read_byte_count;

    return read_byte_count;
}

COS_ASSUME_NONNULL_END
