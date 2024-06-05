/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/io/CosStream.h"

#include "common/Assert.h"
#include "common/CosError.h"

#include <stddef.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosStream {
    void *context;
    CosStreamFunctions *functions;
};

CosStream *
cos_stream_create(CosStreamFunctions *functions,
                  void *context)
{
    COS_PARAMETER_ASSERT(functions != NULL);
    if (COS_UNLIKELY(!functions)) {
        return NULL;
    }

    CosStream *stream = calloc(1, sizeof(CosStream));
    if (COS_UNLIKELY(!stream)) {
        return NULL;
    }

    stream->functions = functions;
    stream->context = context;

    return stream;
}

void
cos_stream_close(CosStream *stream)
{
    COS_PARAMETER_ASSERT(cos_stream_is_valid(stream));
    if (COS_UNLIKELY(!cos_stream_is_valid(stream))) {
        return;
    }

    if (stream->functions->close_func) {
        stream->functions->close_func(stream->context);
    }

    free(stream);
}

bool
cos_stream_is_valid(const CosStream *stream)
{
    return (stream != NULL && stream->functions != NULL);
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

    const CosStreamFunctions * const functions = stream->functions;
    if (!functions->read_func) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Stream does not support reading"),
                            out_error);
        return 0;
    }

    return functions->read_func(stream->context,
                                buffer,
                                count,
                                out_error);
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

    const CosStreamFunctions * const functions = stream->functions;
    if (!functions->write_func) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Stream does not support writing"),
                            out_error);
        return 0;
    }

    return functions->write_func(stream->context,
                                 buffer,
                                 count,
                                 out_error);
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

    const CosStreamFunctions * const functions = stream->functions;
    if (!functions->seek_func) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Stream does not support seeking"),
                            out_error);
        return false;
    }

    return functions->seek_func(stream->context,
                                offset,
                                whence,
                                out_error);
}

CosStreamOffset
cos_stream_tell(CosStream *stream,
                CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(cos_stream_is_valid(stream));
    if (COS_UNLIKELY(!cos_stream_is_valid(stream))) {
        return 0;
    }

    const CosStreamFunctions * const functions = stream->functions;
    if (!functions->tell_func) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Stream does not support telling"),
                            out_error);
        return 0;
    }

    return functions->tell_func(stream->context,
                                out_error);
}

COS_ASSUME_NONNULL_END
