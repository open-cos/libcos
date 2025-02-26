/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_FILTERS_COS_RUN_LENGTH_FILTER_H
#define LIBCOS_FILTERS_COS_RUN_LENGTH_FILTER_H

#include <libcos/common/CosDefines.h>
#include <libcos/filters/CosFilter.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef struct CosRunLengthFilterContext CosRunLengthFilterContext;

/**
 * @brief The run-length encoding filter.
 */
struct CosRunLengthFilter {
    /**
     * @brief The base filter.
     */
    CosFilter base;

    CosRunLengthFilterContext *context;
};

/**
 * @brief Creates a new run-length encoding filter.
 *
 * @return The new run-length encoding filter, or @c NULL if memory allocation failed.
 */
CosRunLengthFilter * COS_Nullable
cos_run_length_filter_create(void)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_stream_close);

/**
 * @brief Initializes a run-length encoding filter.
 *
 * @param run_length_filter The run-length encoding filter to be initialized.
 */
bool
cos_run_length_filter_init(CosRunLengthFilter *run_length_filter);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_FILTERS_COS_RUN_LENGTH_FILTER_H */
