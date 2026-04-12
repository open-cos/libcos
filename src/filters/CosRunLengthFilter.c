/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/filters/CosRunLengthFilter.h"

#include "common/Assert.h"
#include "common/CosError.h"
#include "common/CosMacros.h"

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

enum CosRunLengthConstants {

    COS_RUN_LENGTH_LITERAL_INDICATOR_MAX = 127,

    /**
     * @brief The base value for computing the repeat count of a copy run.
     *
     * The repeat count is @c COS_RUN_LENGTH_COPY_COUNT_BASE minus the run-length indicator byte.
     */
    COS_RUN_LENGTH_COPY_COUNT_BASE = 257,

    /**
     * @brief The end-of-data marker.
     */
    COS_RUN_LENGTH_EOD = 128,
};

/**
 * @brief The type of a run.
 */
typedef enum CosRunLength_RunType {
    /**
     * @brief No run type.
     */
    CosRunLength_RunType_None = 0,

    /**
     * @brief A literal run, where bytes are copied as-is.
     */
    CosRunLength_RunType_Literal,

    /**
     * @brief A copy run, where a byte is repeated.
     */
    CosRunLength_RunType_Copy,

} CosRunLength_RunType;

struct CosRunLengthFilterContext {
    /**
     * @brief The current run type.
     */
    CosRunLength_RunType current_run_type;

    /**
     * @brief The remaining length of the current run.
     *
     * This is only valid when the current run type is @c CosRunLength_RunType_Literal or
     * @c CosRunLength_RunType_Copy .
     */
    uint8_t remaining_run_length;

    /**
     * @brief The repeated byte of the current copy-run.
     *
     * This is only valid when the current run type is @c CosRunLength_RunType_Copy .
     */
    unsigned char run_repeated_byte;
};

// Private function prototypes

static bool
cos_run_length_filter_init_(CosRunLengthFilter *run_length_filter);

static void
cos_run_length_filter_close_(CosFilter *filter);

static size_t
cos_run_length_fill_decode_buffer_(CosFilter *filter,
                                   CosError * COS_Nullable error);

static size_t
cos_run_length_decode_run_(CosRunLengthFilter *run_length_filter,
                           CosError * COS_Nullable error);

static size_t
cos_run_length_decode_literal_run_(CosRunLengthFilter *run_length_filter,
                                   CosError * COS_Nullable error);

static size_t
cos_run_length_decode_copy_run_(CosRunLengthFilter *run_length_filter,
                                CosError * COS_Nullable error);

CosRunLengthFilter *
cos_run_length_filter_create(void)
{
    CosRunLengthFilter * const run_length_filter = calloc(1, sizeof(CosRunLengthFilter));
    if (COS_UNLIKELY(!run_length_filter)) {
        return NULL;
    }

    if (COS_UNLIKELY(!cos_run_length_filter_init_(run_length_filter))) {
        goto failure;
    }

    return run_length_filter;

failure:
    if (run_length_filter) {
        free(run_length_filter);
    }
    return NULL;
}

static bool
cos_run_length_filter_init_(CosRunLengthFilter *run_length_filter)
{
    COS_IMPL_PARAM_CHECK(run_length_filter != NULL);

    static const CosFilterFunctions run_length_filter_functions_ = {
        .decode_func = &cos_run_length_fill_decode_buffer_,
        .encode_func = NULL,
        .close_func  = &cos_run_length_filter_close_,
    };

    cos_filter_init(&(run_length_filter->base),
                    &run_length_filter_functions_);

    CosRunLengthFilterContext * const context = calloc(1, sizeof(CosRunLengthFilterContext));
    if (COS_UNLIKELY(!context)) {
        return false;
    }

    run_length_filter->context = context;

    return true;
}

// Private function implementations

static void
cos_run_length_filter_close_(CosFilter *filter)
{
    COS_IMPL_PARAM_CHECK(filter != NULL);

    CosRunLengthFilter * const run_length_filter = (CosRunLengthFilter *)filter;
    if (run_length_filter->context) {
        free(run_length_filter->context);
        run_length_filter->context = NULL;
    }
}

static size_t
cos_run_length_fill_decode_buffer_(CosFilter *filter,
                                   CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(filter != NULL);

    CosRunLengthFilter * const run_length_filter = (CosRunLengthFilter *)filter;

    size_t total_read = 0;

    while (total_read < COS_FILTER_BUFFER_SIZE) {
        size_t read_count = cos_run_length_decode_run_(run_length_filter, error);
        if (read_count == 0) {
            break;
        }

        total_read += read_count;
    }

    return total_read;
}

static size_t
cos_run_length_decode_run_(CosRunLengthFilter *run_length_filter,
                           CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(run_length_filter != NULL);

    CosStream * const source_stream = run_length_filter->base.source;
    if (COS_UNLIKELY(!source_stream)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "No source stream"),
                            error);
        return 0;
    }

    CosFilterBuffer * const buf = &run_length_filter->base.buffer;
    const size_t buffer_size = COS_FILTER_BUFFER_SIZE;

    size_t total_read_count = 0;

    if (run_length_filter->context->current_run_type == CosRunLength_RunType_None ||
        run_length_filter->context->remaining_run_length == 0) {
        // Read the next run-length indicator.
        unsigned char run_length_indicator = 0;
        if (cos_stream_read(source_stream, &run_length_indicator, 1, error) == 0) {
            // End of data reached unexpectedly.
            buf->eod = true;
            return total_read_count;
        }

        if (run_length_indicator == COS_RUN_LENGTH_EOD) {
            // End of data reached.
            run_length_filter->context->current_run_type = CosRunLength_RunType_None;
            buf->eod = true;
            return total_read_count;
        }

        if (run_length_indicator <= COS_RUN_LENGTH_LITERAL_INDICATOR_MAX) {
            // Copy the next length+1 (1 to 128) bytes directly to the output buffer.
            run_length_filter->context->current_run_type = CosRunLength_RunType_Literal;
            run_length_filter->context->remaining_run_length = (uint8_t)(run_length_indicator + 1);
        }
        else {
            // Copy the next byte 257-length (2 to 128) times.
            run_length_filter->context->current_run_type = CosRunLength_RunType_Copy;
            run_length_filter->context->remaining_run_length = (uint8_t)(COS_RUN_LENGTH_COPY_COUNT_BASE - run_length_indicator);

            unsigned char repeated_byte = 0;
            if (cos_stream_read(source_stream, &repeated_byte, 1, error) == 0) {
                // End of data reached unexpectedly.
                buf->eod = true;
                return total_read_count;
            }

            run_length_filter->context->run_repeated_byte = repeated_byte;
        }
    }

    COS_ASSERT(run_length_filter->context->current_run_type != CosRunLength_RunType_None,
               "Invalid run type");

    if (run_length_filter->context->remaining_run_length > 0 &&
        buf->length < buffer_size) {
        size_t read_count = 0;
        if (run_length_filter->context->current_run_type == CosRunLength_RunType_Literal) {
            read_count = cos_run_length_decode_literal_run_(run_length_filter,
                                                            error);
        }
        else { // CosRunLength_RunType_Copy
            read_count = cos_run_length_decode_copy_run_(run_length_filter,
                                                         error);
        }

        if (read_count == 0) {
            return total_read_count;
        }
        total_read_count += read_count;
    }

    return total_read_count;
}

static size_t
cos_run_length_decode_literal_run_(CosRunLengthFilter *run_length_filter,
                                   CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(run_length_filter != NULL);
    COS_IMPL_PARAM_CHECK(run_length_filter->context->current_run_type == CosRunLength_RunType_Literal);

    CosStream * const source_stream = run_length_filter->base.source;
    COS_ASSERT(source_stream != NULL, "No source stream");

    CosFilterBuffer * const buf = &run_length_filter->base.buffer;

    unsigned char * const output_buffer = buf->data + buf->length;

    // Read as much of the literal run as will fit in the buffer.
    const size_t max_read_count = COS_MIN(run_length_filter->context->remaining_run_length,
                                          COS_FILTER_BUFFER_SIZE - buf->length);

    const size_t total_read_count = cos_stream_read(source_stream,
                                                    output_buffer,
                                                    max_read_count,
                                                    error);
    if (COS_UNLIKELY(total_read_count == 0)) {
        return total_read_count;
    }

    buf->length += total_read_count;
    run_length_filter->context->remaining_run_length -= (uint8_t)total_read_count;

    return total_read_count;
}

static size_t
cos_run_length_decode_copy_run_(CosRunLengthFilter *run_length_filter,
                                CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(run_length_filter != NULL);
    COS_IMPL_PARAM_CHECK(run_length_filter->context->current_run_type == CosRunLength_RunType_Copy);

    (void)error;

    CosFilterBuffer * const buf = &run_length_filter->base.buffer;

    unsigned char * const output_buffer = buf->data + buf->length;

    const size_t copy_count = COS_MIN(run_length_filter->context->remaining_run_length,
                                      COS_FILTER_BUFFER_SIZE - buf->length);

    memset(output_buffer,
           run_length_filter->context->run_repeated_byte,
           copy_count);

    buf->length += copy_count;
    run_length_filter->context->remaining_run_length -= (uint8_t)copy_count;

    return copy_count;
}

COS_ASSUME_NONNULL_END
