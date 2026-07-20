/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "filters/flate/CosFlateDecoder.h"

#include "common/Assert.h"
#include "common/CosError.h"

#include <libcos/common/CosMacros.h>

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include <zlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosFlateDecoder {
    z_stream stream;
};

CosFlateDecoder *
cos_flate_decoder_create(void)
{
    CosFlateDecoder * const decoder = cos_calloc(1, sizeof(CosFlateDecoder));
    if (COS_UNLIKELY(!decoder)) {
        return NULL;
    }

    // A zeroed z_stream selects zlib's default allocators.
    if (inflateInit(&decoder->stream) != Z_OK) {
        cos_free(decoder);
        return NULL;
    }

    return decoder;
}

void
cos_flate_decoder_destroy(CosFlateDecoder *decoder)
{
    COS_IMPL_PARAM_CHECK(decoder != NULL);

    (void)inflateEnd(&decoder->stream);
    cos_free(decoder);
}

CosFlateStatus
cos_flate_decoder_inflate(CosFlateDecoder *decoder,
                          const unsigned char *in,
                          size_t *in_len,
                          unsigned char *out,
                          size_t *out_len,
                          CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(decoder != NULL);
    COS_IMPL_PARAM_CHECK(in != NULL);
    COS_IMPL_PARAM_CHECK(in_len != NULL);
    COS_IMPL_PARAM_CHECK(out != NULL);
    COS_IMPL_PARAM_CHECK(out_len != NULL);

    // zlib's avail_in / avail_out are uInt; clamp the exposed window so large
    // size_t values cannot overflow. The caller feeds fixed small chunks.
    const uInt in_avail = (uInt)COS_MIN(*in_len, (size_t)UINT_MAX);
    const uInt out_avail = (uInt)COS_MIN(*out_len, (size_t)UINT_MAX);

    z_stream * const stream = &decoder->stream;
    stream->next_in = (Bytef *)(uintptr_t)in;
    stream->avail_in = in_avail;
    stream->next_out = (Bytef *)out;
    stream->avail_out = out_avail;

    const int ret = inflate(stream, Z_NO_FLUSH);

    *in_len = in_avail - stream->avail_in;
    *out_len = out_avail - stream->avail_out;

    switch (ret) {
        case Z_STREAM_END:
            return CosFlateStatus_Done;

        case Z_OK:
            // Output filled means there may be more to produce; otherwise all
            // supplied input was consumed and more is needed.
            return (stream->avail_out == 0) ? CosFlateStatus_HasOutput
                                            : CosFlateStatus_NeedInput;

        case Z_BUF_ERROR:
            // No progress was possible: more input (or output) is required.
            return CosFlateStatus_NeedInput;

        default:
            COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_SYNTAX,
                                               (stream->msg != NULL)
                                                   ? stream->msg
                                                   : "Flate decode error"),
                                out_error);
            return CosFlateStatus_Error;
    }
}

COS_ASSUME_NONNULL_END
