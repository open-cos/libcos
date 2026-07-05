/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/filters/CosFlateFilter.h"

#include "common/Assert.h"
#include "common/CosError.h"
#include "filters/flate/CosFlateDecoder.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

enum {
    /**
     * @brief The size of the staging buffer holding compressed bytes read from
     * the source stream before they are fed to the decoder.
     */
    COS_FLATE_INPUT_STAGING_SIZE = 512,
};

struct CosFlateFilterContext {
    /**
     * @brief The flate-provider decoder that performs the actual inflate work.
     */
    CosFlateDecoder *decoder;

    /**
     * @brief Compressed bytes read from the source stream, awaiting decode.
     */
    unsigned char in_buffer[COS_FLATE_INPUT_STAGING_SIZE];

    /**
     * @brief The number of valid bytes in @c in_buffer.
     */
    size_t in_length;

    /**
     * @brief The offset of the next byte in @c in_buffer to feed the decoder.
     */
    size_t in_position;

    /**
     * @brief Whether the source stream has been fully consumed.
     */
    bool source_eof;

    /**
     * @brief Whether the decoder has reported the end of the zlib stream.
     */
    bool done;
};

// Private function prototypes

static bool
cos_flate_filter_init_(CosFlateFilter *flate_filter);

static void
cos_flate_filter_close_(CosFilter *filter);

static size_t
cos_flate_fill_decode_buffer_(CosFilter *filter,
                              CosError * COS_Nullable error);

// Public function implementations

CosFlateFilter *
cos_flate_filter_create(void)
{
    CosFlateFilter * const flate_filter = calloc(1, sizeof(CosFlateFilter));
    if (COS_UNLIKELY(!flate_filter)) {
        goto failure;
    }

    if (COS_UNLIKELY(!cos_flate_filter_init_(flate_filter))) {
        goto failure;
    }

    return flate_filter;

failure:
    if (flate_filter) {
        free(flate_filter);
    }
    return NULL;
}

static bool
cos_flate_filter_init_(CosFlateFilter *flate_filter)
{
    COS_IMPL_PARAM_CHECK(flate_filter != NULL);

    static const CosFilterFunctions flate_filter_functions_ = {
        .decode_func = &cos_flate_fill_decode_buffer_,
        .encode_func = NULL,
        .close_func = &cos_flate_filter_close_,
    };

    CosFlateFilterContext *context = NULL;
    CosFlateDecoder *decoder = NULL;

    cos_filter_init(&(flate_filter->base),
                    &flate_filter_functions_);

    context = calloc(1, sizeof(CosFlateFilterContext));
    if (COS_UNLIKELY(!context)) {
        goto failure;
    }

    decoder = cos_flate_decoder_create();
    if (COS_UNLIKELY(!decoder)) {
        goto failure;
    }

    context->decoder = decoder;
    flate_filter->context = context;

    return true;

failure:
    if (context) {
        free(context);
    }
    if (decoder) {
        cos_flate_decoder_destroy(decoder);
    }
    return false;
}

// Private function implementations

static void
cos_flate_filter_close_(CosFilter *filter)
{
    COS_IMPL_PARAM_CHECK(filter != NULL);

    CosFlateFilter * const flate_filter = (CosFlateFilter *)filter;
    if (flate_filter->context) {
        if (flate_filter->context->decoder) {
            cos_flate_decoder_destroy(flate_filter->context->decoder);
        }
        free(flate_filter->context);
    }
}

static size_t
cos_flate_fill_decode_buffer_(CosFilter *filter,
                              CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(filter != NULL);

    CosFlateFilter * const flate_filter = (CosFlateFilter *)filter;
    CosFlateFilterContext * const context = flate_filter->context;

    CosStream * const source_stream = filter->source;
    if (COS_UNLIKELY(!source_stream)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "No source stream"),
                            error);
        return 0;
    }

    CosFilterBuffer * const buf = &filter->buffer;

    while (buf->length < COS_FILTER_BUFFER_SIZE && !context->done) {
        // Ensure the staging buffer has compressed bytes to feed the decoder.
        if (context->in_position >= context->in_length) {
            if (context->source_eof) {
                // The decoder needs more input, but the source is exhausted.
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_SYNTAX,
                                                   "Unexpected end of flate data"),
                                    error);
                buf->eod = true;
                break;
            }

            const size_t read_count = cos_stream_read(source_stream,
                                                      context->in_buffer,
                                                      COS_FLATE_INPUT_STAGING_SIZE,
                                                      error);
            context->in_length = read_count;
            context->in_position = 0;
            if (read_count == 0) {
                context->source_eof = true;
            }
            continue;
        }

        size_t consumed = context->in_length - context->in_position;
        size_t produced = COS_FILTER_BUFFER_SIZE - buf->length;

        const CosFlateStatus status =
            cos_flate_decoder_inflate(context->decoder,
                                      context->in_buffer + context->in_position,
                                      &consumed,
                                      buf->data + buf->length,
                                      &produced,
                                      error);

        context->in_position += consumed;
        buf->length += produced;

        switch (status) {
            case CosFlateStatus_Done:
                context->done = true;
                buf->eod = true;
                break;

            case CosFlateStatus_Error:
                // The decoder has set the error; stop producing output.
                buf->eod = true;
                return buf->length;

            case CosFlateStatus_NeedInput:
            case CosFlateStatus_HasOutput:
                // Loop to refill input or, when the output buffer is full,
                // exit via the loop condition.
                break;
        }
    }

    return buf->length;
}

COS_ASSUME_NONNULL_END
