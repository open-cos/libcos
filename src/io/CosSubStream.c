/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/io/CosSubStream.h"

#include "common/Assert.h"
#include "common/CosCheckedArith.h"

#include <libcos/common/CosError.h>
#include <libcos/common/memory/CosMemory.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

static size_t
cos_sub_stream_read_(CosStream *stream,
                     void *buffer,
                     size_t count,
                     CosError * COS_Nullable out_error);

static bool
cos_sub_stream_seek_(CosStream *stream,
                     CosStreamOffset offset,
                     CosStreamOffsetWhence whence,
                     CosError * COS_Nullable out_error);

static CosStreamOffset
cos_sub_stream_tell_(CosStream *stream,
                     CosError * COS_Nullable out_error);

static bool
cos_sub_stream_eof_(CosStream *stream);

static void
cos_sub_stream_close_(CosStream *stream);

CosStream *
cos_sub_stream_create(CosStream *source,
                      CosStreamOffset base_offset,
                      size_t length,
                      bool source_owner,
                      CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(source != NULL);
    if (COS_UNLIKELY(!source)) {
        return NULL;
    }

    if (!cos_stream_can_seek(source)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Sub-stream requires a seekable source"),
                            out_error);
        return NULL;
    }

    CosSubStream *sub_stream = cos_calloc(1, sizeof(CosSubStream));
    if (COS_UNLIKELY(!sub_stream)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to allocate sub-stream"),
                            out_error);
        return NULL;
    }

    sub_stream->source = source;
    sub_stream->source_owner = source_owner;
    sub_stream->base_offset = base_offset;
    sub_stream->length = length;
    sub_stream->position = 0;

    const CosStreamFunctions stream_functions = {
        .read_func = &cos_sub_stream_read_,
        .write_func = NULL,
        .seek_func = &cos_sub_stream_seek_,
        .tell_func = &cos_sub_stream_tell_,
        .eof_func = &cos_sub_stream_eof_,
        .close_func = &cos_sub_stream_close_,
    };

    cos_stream_init(&(sub_stream->base),
                    &stream_functions);

    return &(sub_stream->base);
}

static size_t
cos_sub_stream_read_(CosStream *stream,
                     void *buffer,
                     size_t count,
                     CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(stream != NULL);
    COS_API_PARAM_CHECK(buffer != NULL);
    if (count == 0) {
        return 0;
    }

    CosSubStream * const sub_stream = (CosSubStream *)stream;

    const size_t length = sub_stream->length;
    const size_t position = sub_stream->position;

    if (position >= length) {
        return 0;
    }

    const size_t remaining = length - position;
    const size_t to_read = (count < remaining) ? count : remaining;

    // Position the source at the window offset before reading, so a shared source can back
    // several sub-streams. Both terms are bounded by the size of a real file, so the sum
    // cannot overflow in practice; it is checked so that a future caller constructing a
    // window at an arbitrary base offset cannot make it do so silently.
    CosStreamOffset source_position;
    CosStreamOffset source_offset;
    if (cos_ckd_size_to_off_(&source_position, position) ||
        cos_ckd_add_off_(&source_offset, sub_stream->base_offset, source_position)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_OUT_OF_RANGE,
                                           "Sub-stream window extends beyond the offset range"),
                            out_error);
        return 0;
    }

    if (!cos_stream_seek(sub_stream->source,
                         source_offset,
                         CosStreamOffsetWhence_Set,
                         out_error)) {
        return 0;
    }

    const size_t read_count = cos_stream_read(sub_stream->source,
                                              buffer,
                                              to_read,
                                              out_error);
    sub_stream->position += read_count;

    return read_count;
}

static bool
cos_sub_stream_seek_(CosStream *stream,
                     CosStreamOffset offset,
                     CosStreamOffsetWhence whence,
                     CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(stream != NULL);

    CosSubStream * const sub_stream = (CosSubStream *)stream;

    const size_t length = sub_stream->length;
    const size_t position = sub_stream->position;

    /*
     * The offset is range-checked before being narrowed to size_t, and negated
     * through the checked helper: casting first truncates wherever size_t is
     * the narrower type, and negating COS_STREAM_OFFSET_MIN is undefined.
     * cos_memory_stream_seek_ is written the same way.
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
                     magnitude > length) {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_OUT_OF_RANGE,
                                                   "Stream offset out of range"),
                                    out_error);
                return false;
            }

            sub_stream->position = magnitude;
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
                sub_stream->position = position - magnitude;
            }
            else {
                // position <= length is an invariant, so the subtraction here
                // cannot wrap.
                if (cos_ckd_off_to_size_(&magnitude, offset) ||
                    magnitude > length - position) {
                    COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_OUT_OF_RANGE,
                                                       "Stream offset out of range"),
                                        out_error);
                    return false;
                }
                sub_stream->position = position + magnitude;
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

            if (cos_ckd_sub_off_(&negated, 0, offset) ||
                cos_ckd_off_to_size_(&magnitude, negated) ||
                magnitude > length) {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_OUT_OF_RANGE,
                                                   "Stream offset out of range"),
                                    out_error);
                return false;
            }

            sub_stream->position = length - magnitude;
        } break;
    }

    return true;
}

static CosStreamOffset
cos_sub_stream_tell_(CosStream *stream,
                     COS_ATTR_UNUSED CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(stream != NULL);

    CosSubStream * const sub_stream = (CosSubStream *)stream;

    return (CosStreamOffset)sub_stream->position;
}

static bool
cos_sub_stream_eof_(CosStream *stream)
{
    COS_IMPL_PARAM_CHECK(stream != NULL);

    CosSubStream * const sub_stream = (CosSubStream *)stream;

    return sub_stream->position >= sub_stream->length;
}

static void
cos_sub_stream_close_(CosStream *stream)
{
    COS_IMPL_PARAM_CHECK(stream != NULL);

    CosSubStream * const sub_stream = (CosSubStream *)stream;

    if (sub_stream->source_owner) {
        cos_stream_close(sub_stream->source);
    }
}

COS_ASSUME_NONNULL_END
