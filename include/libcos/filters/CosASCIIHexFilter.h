/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_FILTERS_COS_ASCII_HEX_FILTER_H
#define LIBCOS_FILTERS_COS_ASCII_HEX_FILTER_H

#include <libcos/common/CosDefines.h>
#include <libcos/filters/CosFilter.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef struct CosASCIIHexFilterContext CosASCIIHexFilterContext;

struct CosASCIIHexFilter {
    /**
     * @brief The inherited filter.
     */
    CosFilter base;

    /**
     * @brief The private ASCII hexadecimal filter context.
     */
    CosASCIIHexFilterContext *context;
};

/**
 * @brief Creates a new ASCII hexadecimal filter.
 *
 * @return The new ASCII hexadecimal filter, or @c NULL if memory allocation failed.
 */
CosASCIIHexFilter * COS_Nullable
cos_ascii_hex_filter_create(void)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_stream_close);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_FILTERS_COS_ASCII_HEX_FILTER_H */
