/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_FILTERS_COS_FILTER_H
#define LIBCOS_FILTERS_COS_FILTER_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/io/CosStream.h>
#include <libcos/parse/CosParserOptions.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

enum {
    /**
     * The size of a filter's internal data buffer.
     */
    COS_FILTER_BUFFER_SIZE = 256,
};

/**
 * Options governing how a decode filter behaves where the specification and
 * real implementations disagree. Bundled so the settings travel as one unit
 * from the parser, through the stream object, to the filter.
 */
typedef struct CosFilterOptions {
    /**
     * How a missing or malformed end-of-data marker is reported, governed by
     * @c CosStrictGroup_FilterEndOfData . Defaults to @c CosStrictLevel_Off .
     */
    CosStrictLevel eod_strict_level;

    /**
     * Where end-of-data deviations are reported, or @c NULL for the default
     * handler.
     */
    CosDiagnosticHandler * COS_Nullable diagnostic_handler;
} CosFilterOptions;

/**
 * Returns the default filter options: @c CosStrictLevel_Off (matching the
 * @c CosStrictGroup_FilterEndOfData default) and the default diagnostic handler.
 *
 * @return The default filter options.
 */
COS_API CosFilterOptions
cos_filter_options_make_default(void);

struct CosFilterBuffer {
    unsigned char data[COS_FILTER_BUFFER_SIZE];
    size_t length;
    size_t index;
    bool eod;
};

struct CosFilterFunctions {
    /**
     * Fills @c filter->buffer with decoded bytes from the filter's source
     * stream. Sets @c filter->buffer.eod when the end-of-data marker is
     * consumed. Sets @c filter->buffer.length to the number of bytes placed.
     * Returns the number of bytes placed in the buffer.
     * @c NULL means the filter is write-only.
     */
    size_t (* COS_Nullable decode_func)(CosFilter *filter,
                                        CosError * COS_Nullable out_error)
        COS_ATTR_ACCESS_WRITE_ONLY(2);

    /**
     * Encodes @p count bytes from @p input and writes encoded output to the
     * filter's source stream (acting as a sink). Returns the number of bytes
     * consumed from @p input.
     * @c NULL means the filter is read-only.
     */
    size_t (* COS_Nullable encode_func)(CosFilter *filter,
                                        const void *input,
                                        size_t count,
                                        CosError * COS_Nullable out_error)
        COS_ATTR_ACCESS_READ_ONLY_SIZE(2, 3)
        COS_ATTR_ACCESS_WRITE_ONLY(4);

    /**
     * Subclass-specific teardown (free context only). Called by the base
     * close function before base-level cleanup. @c NULL if nothing to free.
     */
    void (* COS_Nullable close_func)(CosFilter *filter);
};

struct CosFilter {
    CosStream base;

    CosStream * COS_Nullable source;

    CosFilterFunctions filter_functions;

    CosFilterBuffer buffer;

    /**
     * Options governing behaviour where the specification and implementations
     * disagree, such as end-of-data marker strictness.
     */
    CosFilterOptions options;
};

/**
 * @brief Initializes a filter.
 *
 * @param filter The filter to be initialized.
 * @param filter_functions The filter functions.
 */
COS_API void
cos_filter_init(CosFilter *filter,
                const CosFilterFunctions *filter_functions);

/**
 * @brief Deinitializes a filter.
 *
 * @param filter The filter.
 */
COS_API void
cos_filter_deinit(CosFilter *filter);

/**
 * @brief Attaches a source stream to a filter.
 *
 * @param filter The filter.
 * @param source The source stream.
 */
COS_API void
cos_filter_attach_source(CosFilter *filter,
                         CosStream *source)
    COS_OWNERSHIP_HOLDS(2);

/**
 * @brief Detaches the source stream from a filter.
 *
 * @param filter The filter.
 */
COS_API void
cos_filter_detach_source(CosFilter *filter);

/**
 * Sets the filter's behaviour options.
 *
 * @param filter The filter.
 * @param options The options to apply, or @c NULL for the defaults.
 */
COS_API void
cos_filter_set_options_(CosFilter *filter,
                        const CosFilterOptions * COS_Nullable options);

/**
 * Reports a missing or malformed end-of-data marker on the filter.
 *
 * Applies the level set by @c cos_filter_set_eod_reporting_ : silent at
 * @c CosStrictLevel_Off , a warning at @c CosStrictLevel_Warn , and an error
 * plus a @c COS_ERROR_SYNTAX propagation at @c CosStrictLevel_Error .
 *
 * @param filter The filter.
 * @param message The message to report; must outlive the call.
 * @param out_error Set if the deviation escalates to an error.
 *
 * @return @c true if decoding may continue, @c false if it must abort.
 */
COS_API bool
cos_filter_report_end_of_data_(CosFilter *filter,
                               const char *message,
                               CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_FILTERS_COS_FILTER_H */
