/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/filters/CosFilter.h"

#include "common/Assert.h"

#include <libcos/common/CosMacros.h>

#include <string.h>

COS_ASSUME_NONNULL_BEGIN

// Private function prototypes

static size_t
cos_filter_read_(CosStream *stream,
                 void *buffer,
                 size_t count,
                 CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(4);

static size_t
cos_filter_write_(CosStream *stream,
                  const void *buffer,
                  size_t count,
                  CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(4);

static bool
cos_filter_eof_(CosStream *stream);

static void
cos_filter_close_(CosStream *stream);

static const CosStreamFunctions cos_filter_stream_functions_ = {
    .read_func = &cos_filter_read_,
    .write_func = &cos_filter_write_,
    .eof_func = &cos_filter_eof_,
    .close_func = &cos_filter_close_,
};

// Public function implementations

void
cos_filter_init(CosFilter *filter,
                const CosFilterFunctions *filter_functions)
{
    COS_API_PARAM_CHECK(filter != NULL);
    COS_API_PARAM_CHECK(filter_functions != NULL);
    if (COS_UNLIKELY(!filter || !filter_functions)) {
        return;
    }

    cos_stream_init((CosStream *)filter,
                    &cos_filter_stream_functions_);

    filter->source = NULL;
    filter->filter_functions = *filter_functions;
    filter->buffer = (CosFilterBuffer){0};
}

void
cos_filter_deinit(CosFilter *filter)
{
    COS_API_PARAM_CHECK(filter != NULL);
    if (COS_UNLIKELY(!filter)) {
        return;
    }

    CosStream *source = filter->source;
    if (source) {
        cos_stream_close(source);
    }
}

void
cos_filter_attach_source(CosFilter *filter,
                         CosStream *source)
{
    COS_API_PARAM_CHECK(filter != NULL);
    COS_API_PARAM_CHECK(source != NULL);
    if (COS_UNLIKELY(!filter || !source)) {
        return;
    }

    filter->source = source;
}

void
cos_filter_detach_source(CosFilter *filter)
{
    COS_API_PARAM_CHECK(filter != NULL);
    if (COS_UNLIKELY(!filter)) {
        return;
    }

    filter->source = NULL;
}

// Private function implementations

static size_t
cos_filter_read_(CosStream *stream,
                 void *buffer,
                 size_t count,
                 CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(stream != NULL);
    COS_IMPL_PARAM_CHECK(buffer != NULL);

    CosFilter * const filter = (CosFilter *)stream;

    // Check if the filter supports decoding.
    if (!filter->filter_functions.decode_func) {
        return 0;
    }

    unsigned char * const out_buffer = (unsigned char *)buffer;
    size_t total_read = 0;

    while (total_read < count) {
        if (filter->buffer.index >= filter->buffer.length) {
            if (filter->buffer.eod) {
                // The end-of-data marker has been set; no more data will be read.
                break;
            }

            filter->buffer.length = 0;
            filter->buffer.index = 0;
            const size_t filled = filter->filter_functions.decode_func(filter, out_error);
            if (filled == 0 || filter->buffer.length == 0) {
                // No more data to read.
                break;
            }
        }

        // Fill the output buffer with as many bytes that are available in the
        // internal buffer, subject to the size of the output buffer.
        const size_t read_count = COS_MIN(filter->buffer.length - filter->buffer.index,
                                          count - total_read);
        memcpy(out_buffer + total_read,
               filter->buffer.data + filter->buffer.index,
               read_count);
        filter->buffer.index += read_count;
        total_read += read_count;
    }

    return total_read;
}

static size_t
cos_filter_write_(CosStream *stream,
                  const void *buffer,
                  size_t count,
                  CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(stream != NULL);
    COS_IMPL_PARAM_CHECK(buffer != NULL);

    CosFilter * const filter = (CosFilter *)stream;

    if (!filter->filter_functions.encode_func) {
        return 0;
    }

    return filter->filter_functions.encode_func(filter, buffer, count, out_error);
}

static bool
cos_filter_eof_(CosStream *stream)
{
    COS_IMPL_PARAM_CHECK(stream != NULL);

    const CosFilter * const filter = (const CosFilter *)stream;

    return (filter->buffer.eod) && (filter->buffer.index >= filter->buffer.length);
}

static void
cos_filter_close_(CosStream *stream)
{
    COS_IMPL_PARAM_CHECK(stream != NULL);

    CosFilter * const filter = (CosFilter *)stream;

    if (filter->filter_functions.close_func) {
        filter->filter_functions.close_func(filter);
    }

    cos_filter_deinit(filter);
}

COS_ASSUME_NONNULL_END
