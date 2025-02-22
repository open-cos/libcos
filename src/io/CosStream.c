/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/io/CosStream.h"

#include "common/Assert.h"

#include "libcos/common/CosError.h"
#include "libcos/common/CosMacros.h"

#include <stddef.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

CosStream *
cos_stream_create(const CosStreamFunctions *functions)
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

    stream->functions = *functions;
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

    return stream->functions.read_func(stream,
                                       buffer,
                                       count,
                                       out_error);
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

    return stream->functions.write_func(stream,
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

    return stream->functions.seek_func(stream,
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
                                           "Stream does not support getting position"),
                            out_error);
        return -1;
    }

    return stream->functions.tell_func(stream, out_error);
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

COS_ASSUME_NONNULL_END
