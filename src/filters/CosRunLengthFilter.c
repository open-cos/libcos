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
     * @brief The buffer used to store decoded or encoded data.
     */
    unsigned char buffer[256];

    /**
     * @brief The number of valid bytes in @c buffer .
     */
    size_t buffer_length;

    /**
     * @brief The index of the next byte to read from @c buffer .
     */
    size_t buffer_index;

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

    /**
     * @brief Flag to indicate the end of the data stream.
     */
    bool eod;
};

// Private function prototypes

static size_t
cos_run_length_filter_read_(CosStream *stream,
                            void *buffer,
                            size_t count,
                            CosError * COS_Nullable out_error);

static size_t
cos_run_length_filter_write_(CosStream *stream,
                             const void *buffer,
                             size_t count,
                             CosError * COS_Nullable out_error);

static void
cos_run_length_filter_close_(CosStream *stream);

static size_t
cos_run_length_fill_decode_buffer_(CosRunLengthFilter *run_length_filter,
                                   size_t count,
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

    if (COS_UNLIKELY(!cos_run_length_filter_init(run_length_filter))) {
        goto failure;
    }

    return run_length_filter;

failure:
    if (run_length_filter) {
        free(run_length_filter);
    }
    return NULL;
}

bool
cos_run_length_filter_init(CosRunLengthFilter *run_length_filter)
{
    COS_API_PARAM_CHECK(run_length_filter != NULL);

    cos_filter_init(&(run_length_filter->base),
                    &(CosStreamFunctions){
                        .read_func = &cos_run_length_filter_read_,
                        .write_func = &cos_run_length_filter_write_,
                        .close_func = &cos_run_length_filter_close_,
                    });

    CosRunLengthFilterContext * const context = calloc(1, sizeof(CosRunLengthFilterContext));
    if (COS_UNLIKELY(!context)) {
        return false;
    }

    run_length_filter->context = context;

    return true;
}

// Private function implementations

static size_t
cos_run_length_filter_read_(CosStream *stream,
                            void *buffer,
                            size_t count,
                            CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(stream != NULL);
    COS_IMPL_PARAM_CHECK(buffer != NULL);

    CosRunLengthFilter * const run_length_filter = (CosRunLengthFilter *)stream;

    unsigned char * const out_buffer = (unsigned char *)buffer;
    size_t total_read = 0;

    while (total_read < count) {
        if (run_length_filter->context->buffer_index == run_length_filter->context->buffer_length) {
            if (run_length_filter->context->eod) {
                // End of data reached.
                break;
            }

            const size_t decoded_count = cos_run_length_fill_decode_buffer_(run_length_filter,
                                                                            count - total_read,
                                                                            out_error);
            if (decoded_count == 0 || run_length_filter->context->buffer_length == 0) {
                break;
            }
        }

        const size_t read_count = COS_MIN(run_length_filter->context->buffer_length - run_length_filter->context->buffer_index,
                                          count - total_read);
        memcpy(out_buffer + total_read,
               run_length_filter->context->buffer + run_length_filter->context->buffer_index,
               read_count);

        run_length_filter->context->buffer_index += read_count;

        total_read += read_count;
    }

    return total_read;
}

static size_t
cos_run_length_filter_write_(CosStream *stream,
                             const void *buffer,
                             size_t count,
                             CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(stream != NULL);
    COS_IMPL_PARAM_CHECK(buffer != NULL);

    (void)stream;
    (void)buffer;
    (void)count;
    (void)out_error;

    return 0;
}

static void
cos_run_length_filter_close_(CosStream *stream)
{
    COS_IMPL_PARAM_CHECK(stream != NULL);

    CosRunLengthFilter * const run_length_filter = (CosRunLengthFilter *)stream;
    if (run_length_filter->context) {
        free(run_length_filter->context);
    }

    cos_filter_deinit(&(run_length_filter->base));
}

// Private function implementations

static size_t
cos_run_length_fill_decode_buffer_(CosRunLengthFilter *run_length_filter,
                                   size_t count,
                                   CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(run_length_filter != NULL);

    // Reset internal buffer.
    run_length_filter->context->buffer_length = 0;
    run_length_filter->context->buffer_index = 0;

    // Ensure we have a source stream to read from.
    CosStream * const source_stream = run_length_filter->base.source;
    if (COS_UNLIKELY(!source_stream)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "No source stream"),
                            error);
        return 0;
    }

    const size_t buffer_size = COS_ARRAY_SIZE(run_length_filter->context->buffer);

    const size_t max_read_count = COS_MIN(count, buffer_size);
    size_t total_read = 0;

    while (total_read < max_read_count) {
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
    COS_ASSERT(source_stream != NULL, "No source stream");

    const size_t buffer_size = COS_ARRAY_SIZE(run_length_filter->context->buffer);

    size_t total_read_count = 0;

    if (run_length_filter->context->current_run_type == CosRunLength_RunType_None ||
        run_length_filter->context->remaining_run_length == 0) {
        // Read the next run-length indicator.
        unsigned char run_length_indicator = 0;
        if (cos_stream_read(source_stream, &run_length_indicator, 1, error) == 0) {
            // End of data reached unexpectedly.
            run_length_filter->context->eod = true;
            return total_read_count;
        }

        if (run_length_indicator == COS_RUN_LENGTH_EOD) {
            // End of data reached.
            run_length_filter->context->current_run_type = CosRunLength_RunType_None;
            run_length_filter->context->eod = true;
            return total_read_count;
        }

        if (run_length_indicator <= 127) {
            // Copy the next length+1 bytes directly to the output buffer.
            run_length_filter->context->current_run_type = CosRunLength_RunType_Literal;
            run_length_filter->context->remaining_run_length = (uint8_t)(run_length_indicator + 1);
        }
        else {
            // Copy the next byte 257-length times.
            run_length_filter->context->current_run_type = CosRunLength_RunType_Copy;
            run_length_filter->context->remaining_run_length = (uint8_t)(257 - run_length_indicator);

            unsigned char repeated_byte = 0;
            if (cos_stream_read(source_stream, &repeated_byte, 1, error) == 0) {
                // End of data reached unexpectedly.
                run_length_filter->context->eod = true;
                return total_read_count;
            }

            run_length_filter->context->run_repeated_byte = repeated_byte;
        }
    }

    COS_ASSERT(run_length_filter->context->current_run_type != CosRunLength_RunType_None,
               "Invalid run type");

    if (run_length_filter->context->remaining_run_length > 0 &&
        run_length_filter->context->buffer_length < buffer_size) {
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

    unsigned char *output_buffer = run_length_filter->context->buffer + run_length_filter->context->buffer_length;

    // Read as much of the literal run as will fit in the buffer.
    const size_t max_read_count = COS_MIN(run_length_filter->context->remaining_run_length,
                                          COS_ARRAY_SIZE(run_length_filter->context->buffer) - run_length_filter->context->buffer_length);

    const size_t total_read_count = cos_stream_read(source_stream,
                                                    output_buffer,
                                                    max_read_count,
                                                    error);
    if (COS_UNLIKELY(total_read_count == 0)) {
        return total_read_count;
    }

    run_length_filter->context->buffer_length += total_read_count;
    run_length_filter->context->remaining_run_length -= (uint8_t)total_read_count;

    return total_read_count;
}

static size_t
cos_run_length_decode_copy_run_(CosRunLengthFilter *run_length_filter,
                                CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(run_length_filter != NULL);
    COS_IMPL_PARAM_CHECK(run_length_filter->context->current_run_type == CosRunLength_RunType_Copy);

    unsigned char *output_buffer = run_length_filter->context->buffer + run_length_filter->context->buffer_length;

    const size_t copy_count = COS_MIN(run_length_filter->context->remaining_run_length,
                                      COS_ARRAY_SIZE(run_length_filter->context->buffer) - run_length_filter->context->buffer_length);

    memset(output_buffer,
           run_length_filter->context->run_repeated_byte,
           copy_count);

    run_length_filter->context->buffer_length += copy_count;
    run_length_filter->context->remaining_run_length -= (uint8_t)copy_count;

    return copy_count;
}

COS_ASSUME_NONNULL_END
