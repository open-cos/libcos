/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_FILTERS_COS_ASCII_85_FILTER_H
#define LIBCOS_FILTERS_COS_ASCII_85_FILTER_H

#include <libcos/common/CosDefines.h>
#include <libcos/filters/CosFilter.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef struct CosASCII85FilterContext CosASCII85FilterContext;

struct CosASCII85Filter {
    CosFilter base;

    CosASCII85FilterContext *context;
};

/**
 * @brief Creates a new ASCII base-85 filter.
 *
 * @return The new ASCII base-85 filter, or @c NULL if memory allocation failed.
 */
CosASCII85Filter * COS_Nullable
cos_ascii85_filter_create(void)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_stream_close);

/**
 * @brief Initializes an ASCII base-85 filter.
 *
 * @param ascii_85_filter The ASCII base-85 filter to be initialized.
 *
 * @return @c true if the filter was initialized, @c false otherwise.
 */
bool
cos_ascii85_filter_init(CosASCII85Filter *ascii_85_filter);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_FILTERS_COS_ASCII_85_FILTER_H */
