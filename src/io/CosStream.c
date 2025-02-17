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

// Static function prototypes.

static bool
cos_stream_fill_read_buffer_(CosStream *stream,
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

    // Ensure that the stream functions are valid.
    if (!functions->eof_func ||
        !functions->close_func) {
        return NULL;
    }

    stream = calloc(1, sizeof(CosStream));
    if (COS_UNLIKELY(!stream)) {
        goto failure;
    }

    cos_stream_init(stream, functions);

    return stream;

failure:
    if (stream) {
        free(stream);
    }
    return NULL;
}

void
cos_stream_init(CosStream *stream,
                const CosStreamFunctions *functions)
{
    COS_API_PARAM_CHECK(stream != NULL);
    COS_API_PARAM_CHECK(functions != NULL);
    if (COS_UNLIKELY(!stream || !functions)) {
        return;
    }

    unsigned char *buffer = NULL;

    const size_t buffer_capacity = COS_STREAM_BUFFER_CAPACITY;

    buffer = malloc(buffer_capacity);
    if (COS_UNLIKELY(!buffer)) {
        goto failure;
    }

    stream->functions = *functions;

    stream->base_offset = 0;
    stream->physical_position = 0;

    stream->buffer = buffer;
    stream->buffer_capacity = buffer_capacity;

    stream->buffer_position = 0;
    stream->buffer_length = 0;

    stream->buffer_mode = CosStreamBufferMode_Read;

    return;

failure:
    if (buffer) {
        free(buffer);
    }
}

void
cos_stream_close(CosStream *stream)
{
    COS_PARAMETER_ASSERT(cos_stream_is_valid(stream));
    if (COS_UNLIKELY(!cos_stream_is_valid(stream))) {
        return;
    }

    if (stream->functions.close_func) {
        stream->functions.close_func(stream);
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
        if (stream->buffer_length > 0) {
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
        }

        // Re-fill the buffer with more data.
        if (!cos_stream_fill_read_buffer_(stream,
                                          out_error)) {
            // No more data to read.
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

    // Skip doing any work if we don't need to write any data.
    if (COS_UNLIKELY(count == 0)) {
        return 0;
    }

    COS_ASSERT(false, "Not implemented");
    return 0;
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

    const CosStreamOffset current_position = cos_stream_get_position(stream, out_error);
    if (current_position == -1) {
        return false;
    }

    CosStreamOffset absolute_offset = 0;

    // Convert a relative offset to an absolute offset.
    switch (whence) {
        case CosStreamOffsetWhence_Set: {
            absolute_offset = offset;
        } break;

        case CosStreamOffsetWhence_Current: {
            if (offset == 0) {
                // Nothing to do.
                return true;
            }

            // TODO: Check for overflow.
            absolute_offset = current_position + offset;
        } break;

        case CosStreamOffsetWhence_End: {
            COS_ASSERT(false, "Not implemented");
            return false;
        }
    }

    // Ensure that the offset is valid.
    if (absolute_offset < 0) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Seek offset is negative"),
                            out_error);
        return false;
    }

    // Determine the stream buffer's start (absolute) position.
    const CosStreamOffset buffer_start_position = stream->physical_position - (CosStreamOffset)stream->buffer_length;

    // Seek within the buffer if possible.
    if (absolute_offset >= buffer_start_position &&
        absolute_offset < buffer_start_position + (CosStreamOffset)stream->buffer_length) {
        // The offset is within the buffer.
        const CosStreamOffset buffer_relative_position = absolute_offset - buffer_start_position;
        stream->buffer_position = (size_t)buffer_relative_position;
        return true;
    }

    // We actually need to seek in the stream.
    // Since we are seeking in the underlying stream, we need to adjust for the
    // base offset.
    // TODO: Check for overflow.
    const CosStreamOffset underlying_stream_offset = absolute_offset + stream->base_offset;

    // Reset the buffer before seeking.
    stream->buffer_position = 0;
    stream->buffer_length = 0;

    if (!stream->functions.seek_func(stream,
                                     underlying_stream_offset,
                                     CosStreamOffsetWhence_Set,
                                     out_error)) {
        return false;
    }

    // Update the physical position.
    stream->physical_position = underlying_stream_offset;

    return true;
}

CosStreamOffset
cos_stream_get_position(CosStream *stream,
                        CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(cos_stream_is_valid(stream));
    if (COS_UNLIKELY(!cos_stream_is_valid(stream))) {
        return -1;
    }

    size_t position = (size_t)stream->base_offset;

    const size_t buffered_data_length = stream->buffer_length - stream->buffer_position;

    if (stream->buffer_mode == CosStreamBufferMode_Read) {
        position += (size_t)stream->physical_position - buffered_data_length;
    }
    else {
        position += (size_t)stream->physical_position + buffered_data_length;
    }

    return (CosStreamOffset)position;
}

bool
cos_stream_is_at_end(CosStream *stream,
                     CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(stream != NULL);
    if (COS_UNLIKELY(!stream)) {
        return true;
    }

    // Check if EOF has already been detected.
    if ((stream->flags & CosStreamFlag_EOF) != 0) {
        return true;
    }

    if (!stream->functions.eof_func) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Stream does not support checking for end"),
                            out_error);
        return true;
    }

    const bool is_eof = stream->functions.eof_func(stream);
    if (is_eof) {
        stream->flags |= CosStreamFlag_EOF;
    }
    return is_eof;
}

// Static functions.

static bool
cos_stream_fill_read_buffer_(CosStream *stream,
                             CosError * COS_Nullable out_error)
{
    COS_PARAM_ASSERT_INTERNAL(stream != NULL);

    const size_t read_count = stream->buffer_capacity;
    CosError read_error = CosErrorNone;

    const size_t actual_read_count = stream->functions.read_func(stream,
                                                                 stream->buffer,
                                                                 read_count,
                                                                 &read_error);
    // Always update the buffer, even if an error occurred.
    stream->buffer_length = actual_read_count;
    stream->buffer_position = 0;

    // The physical position will have advanced by the number of bytes read.
    // TODO: Check for overflow.
    stream->physical_position += (CosStreamOffset)actual_read_count;

    if (actual_read_count == 0) {
        // Check if we are at the end of the stream.
        if (cos_stream_is_at_end(stream, NULL)) {
            stream->flags |= CosStreamFlag_EOF;
            return false;
        }
    }

    return true;
}

static size_t
cos_stream_read_from_buffer_(CosStream *stream,
                             void *output_buffer,
                             size_t count)
{
    COS_PARAM_ASSERT_INTERNAL(stream != NULL);
    COS_PARAM_ASSERT_INTERNAL(output_buffer != NULL);

    // Check that we have data available in the buffer.
    const size_t buffered_byte_count = stream->buffer_length - stream->buffer_position;
    if (buffered_byte_count == 0) {
        return 0;
    }

    const size_t read_byte_count = COS_MIN(count, buffered_byte_count);

    memcpy(output_buffer,
           stream->buffer + stream->buffer_position,
           read_byte_count);

    // Update the buffer position.
    stream->buffer_position += read_byte_count;

    return read_byte_count;
}

COS_ASSUME_NONNULL_END
