/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_FILTERS_COS_FILTER_H
#define LIBCOS_FILTERS_COS_FILTER_H

#include <libcos/common/CosDefines.h>
#include <libcos/io/CosStream.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosFilter {
    CosStream base;

    CosStream * COS_Nullable source;
};

/**
 * @brief Initializes a filter.
 *
 * @param filter The filter to be initialized.
 * @param functions The stream functions.
 */
void
cos_filter_init(CosFilter *filter,
                const CosStreamFunctions *functions);

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
