/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/io/CosMemoryStream.h"

#include "common/Assert.h"
#include "common/CosCheckedArith.h"

#include <libcos/common/CosError.h>

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

static size_t
cos_memory_stream_read_(CosStream *stream,
                        void *buffer,
                        size_t count,
                        CosError * COS_Nullable out_error);

static size_t
cos_memory_stream_write_(CosStream *stream,
                         const void *buffer,
                         size_t count,
                         CosError * COS_Nullable out_error);

static bool
cos_memory_stream_seek_(CosStream *stream,
                        CosStreamOffset offset,
                        CosStreamOffsetWhence whence,
                        CosError * COS_Nullable out_error);

static CosStreamOffset
cos_memory_stream_tell_(CosStream *stream,
                        CosError * COS_Nullable out_error);

static bool
cos_memory_stream_eof_(CosStream *stream);

static void
cos_memory_stream_close_(CosStream *stream);

CosMemoryStream *
cos_memory_stream_create(void *buffer,
                         size_t size,
                         bool free_buffer)
{
    COS_API_PARAM_CHECK(buffer != NULL);
    if (COS_UNLIKELY(!buffer)) {
        return NULL;
    }

    CosMemoryStream *memory_stream = calloc(1, sizeof(CosMemoryStream));
    if (COS_UNLIKELY(!memory_stream)) {
        goto failure;
    }

    memory_stream->buffer.write = buffer;
    memory_stream->size = size;
    memory_stream->position = 0;
    memory_stream->free_buffer = free_buffer;

    const CosStreamFunctions stream_functions = {
        .read_func = &cos_memory_stream_read_,
        .write_func = &cos_memory_stream_write_,
        .seek_func = &cos_memory_stream_seek_,
        .tell_func = &cos_memory_stream_tell_,
        .eof_func = &cos_memory_stream_eof_,
        .close_func = &cos_memory_stream_close_,
    };

    cos_stream_init(&(memory_stream->base),
                    &stream_functions);

    return memory_stream;

failure:
    if (memory_stream) {
        free(memory_stream);
    }
    return NULL;
}

CosMemoryStream *
cos_memory_stream_create_readonly(const void *buffer,
                                  size_t size)
{
    COS_API_PARAM_CHECK(buffer != NULL);
    if (COS_UNLIKELY(!buffer)) {
        return NULL;
    }

    const CosStreamFunctions stream_functions = {
        .read_func = &cos_memory_stream_read_,
        .write_func = NULL,
        .seek_func = &cos_memory_stream_seek_,
        .tell_func = &cos_memory_stream_tell_,
        .eof_func = &cos_memory_stream_eof_,
        .close_func = &cos_memory_stream_close_,
    };

    CosMemoryStream *memory_stream = calloc(1, sizeof(CosMemoryStream));
    if (COS_UNLIKELY(!memory_stream)) {
        goto failure;
    }

    memory_stream->buffer.read = buffer;
    memory_stream->size = size;
    memory_stream->position = 0;
    memory_stream->free_buffer = false;

    cos_stream_init(&(memory_stream->base),
                    &stream_functions);

    return memory_stream;

failure:
    if (memory_stream) {
        free(memory_stream);
    }
    return NULL;
}

static size_t
cos_memory_stream_read_(CosStream *stream,
                        void *buffer,
                        size_t count,
                        COS_ATTR_UNUSED CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(stream != NULL);
    COS_API_PARAM_CHECK(buffer != NULL);
    if (count == 0) {
        return 0;
    }

    CosMemoryStream * const memory_stream = (CosMemoryStream *)stream;

    const size_t size = memory_stream->size;
    const size_t position = memory_stream->position;

    if (position >= size) {
        return 0;
    }

    const size_t remaining = size - position;
    const size_t read_count = (count < remaining) ? count : remaining;

    const void * const source = memory_stream->buffer.read + position;
    void * const destination = buffer;

    memcpy(destination,
           source,
           read_count);

    memory_stream->position += read_count;

    return read_count;
}

static size_t
cos_memory_stream_write_(CosStream *stream,
                         const void *buffer,
                         size_t count,
                         COS_ATTR_UNUSED CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(stream != NULL);
    COS_API_PARAM_CHECK(buffer != NULL);

    if (count == 0) {
        return 0;
    }

    CosMemoryStream * const memory_stream = (CosMemoryStream *)stream;

    const size_t size = memory_stream->size;
    const size_t position = memory_stream->position;

    const size_t remaining = size - position;
    const size_t write_count = (count < remaining) ? count : remaining;

    const void * const source = buffer;
    void * const destination = memory_stream->buffer.write + position;

    memcpy(destination,
           source,
           write_count);

    return write_count;
}

static bool
cos_memory_stream_seek_(CosStream *stream,
                        CosStreamOffset offset,
                        CosStreamOffsetWhence whence,
                        CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(stream != NULL);

    CosMemoryStream * const memory_stream = (CosMemoryStream *)stream;

    const size_t size = memory_stream->size;
    const size_t position = memory_stream->position;

    /*
     * The offset is a 64-bit signed value while the position is a size_t, so
     * its magnitude is range-checked before being narrowed. Casting first
     * truncates wherever size_t is the narrower type, which silently turns an
     * out-of-range seek into a small in-range one. Negating the offset is
     * likewise routed through the checked helper, because negating
     * COS_STREAM_OFFSET_MIN would itself overflow.
     */
    switch (whence) {
        case CosStreamOffsetWhence_Set: {
            size_t magnitude;

            if (offset < 0) {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                                   "Invalid negative offset"),
                                    out_error);
                return false;
            }
            else if (cos_ckd_off_to_size_(&magnitude, offset) ||
                     magnitude > size) {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_OUT_OF_RANGE,
                                                   "Stream offset out of range"),
                                    out_error);
                return false;
            }

            memory_stream->position = magnitude;
        } break;
        case CosStreamOffsetWhence_Current: {
            CosStreamOffset negated;
            size_t magnitude;

            if (offset < 0) {
                if (cos_ckd_sub_off_(&negated, 0, offset) ||
                    cos_ckd_off_to_size_(&magnitude, negated) ||
                    magnitude > position) {
                    COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_OUT_OF_RANGE,
                                                       "Invalid negative stream position"),
                                        out_error);
                    return false;
                }

                memory_stream->position = position - magnitude;
            }
            else {
                // Compared by subtraction, so the sum cannot wrap before it is
                // tested. position <= size is an invariant of this stream.
                // Seeking to exactly end-of-stream is legal, matching
                // Whence_Set above and cos_sub_stream_seek.
                if (cos_ckd_off_to_size_(&magnitude, offset) ||
                    magnitude > size - position) {
                    COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_OUT_OF_RANGE,
                                                       "Stream offset out of range"),
                                        out_error);
                    return false;
                }

                memory_stream->position = position + magnitude;
            }
        } break;
        case CosStreamOffsetWhence_End: {
            CosStreamOffset negated;
            size_t magnitude;

            if (offset > 0) {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                                   "Invalid positive offset"),
                                    out_error);
                return false;
            }
            else if (cos_ckd_sub_off_(&negated, 0, offset) ||
                     cos_ckd_off_to_size_(&magnitude, negated) ||
                     magnitude > size) {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_OUT_OF_RANGE,
                                                   "Stream offset out of range"),
                                    out_error);
                return false;
            }

            memory_stream->position = size - magnitude;
        } break;
    }

    return true;
}

static CosStreamOffset
cos_memory_stream_tell_(CosStream *stream,
                        COS_ATTR_UNUSED CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(stream != NULL);

    CosMemoryStream * const memory_stream = (CosMemoryStream *)stream;

    return (CosStreamOffset)memory_stream->position;
}

static void
cos_memory_stream_close_(CosStream *stream)
{
    COS_IMPL_PARAM_CHECK(stream != NULL);

    CosMemoryStream * const memory_stream = (CosMemoryStream *)stream;

    if (memory_stream->free_buffer) {
        free(memory_stream->buffer.write);
    }
}

static bool
cos_memory_stream_eof_(CosStream *stream)
{
    COS_IMPL_PARAM_CHECK(stream != NULL);

    CosMemoryStream * const memory_stream = (CosMemoryStream *)stream;

    return memory_stream->position >= memory_stream->size;
}

COS_ASSUME_NONNULL_END
