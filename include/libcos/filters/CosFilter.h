/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_FILTERS_COS_FILTER_H
#define LIBCOS_FILTERS_COS_FILTER_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/io/CosStream.h>

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
};

/**
 * @brief Initializes a filter.
 *
 * @param filter The filter to be initialized.
 * @param filter_functions The filter functions.
 */
void
cos_filter_init(CosFilter *filter,
                const CosFilterFunctions *filter_functions);

/**
 * @brief Deinitializes a filter.
 *
 * @param filter The filter.
 */
void
cos_filter_deinit(CosFilter *filter);

/**
 * @brief Attaches a source stream to a filter.
 *
 * @param filter The filter.
 * @param source The source stream.
 */
void
cos_filter_attach_source(CosFilter *filter,
                         CosStream *source)
    COS_OWNERSHIP_HOLDS(2);

/**
 * @brief Detaches the source stream from a filter.
 *
 * @param filter The filter.
 */
void
cos_filter_detach_source(CosFilter *filter);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_FILTERS_COS_FILTER_H */
