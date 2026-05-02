/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_FILTERS_COS_FLATE_FILTER_H
#define LIBCOS_FILTERS_COS_FLATE_FILTER_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/filters/CosFilter.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef struct CosFlateFilterContext CosFlateFilterContext;

/**
 * @brief The Flate (zlib/DEFLATE) decoding filter.
 *
 * Decodes zlib-format (RFC 1950) compressed data, corresponding to the PDF
 * @c FlateDecode filter. The actual inflate work is delegated to a swappable
 * flate-decoder provider selected at build time.
 */
struct CosFlateFilter {
    /**
     * @brief The base filter.
     */
    CosFilter base;

    CosFlateFilterContext *context;
};

/**
 * @brief Creates a new Flate decoding filter.
 *
 * @return The new Flate filter, or @c NULL if memory allocation failed.
 */
COS_API CosFlateFilter * COS_Nullable
cos_flate_filter_create(void)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_stream_close);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_FILTERS_COS_FLATE_FILTER_H */
