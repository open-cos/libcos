/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_FILTERS_COS_PREDICTOR_FILTER_H
#define LIBCOS_FILTERS_COS_PREDICTOR_FILTER_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/filters/CosFilter.h>

#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef struct CosPredictorFilter CosPredictorFilter;
typedef struct CosPredictorFilterContext CosPredictorFilterContext;

/**
 * @brief A predictor post-processing filter (PDF @c /DecodeParms).
 *
 * Reverses the row prediction applied to a filter's decoded output before the
 * data is consumed. This is an internal building block applied automatically by
 * the stream decode pipeline; it is not a PDF-named filter.
 */
struct CosPredictorFilter {
    CosFilter base;

    CosPredictorFilterContext *context;
};

/**
 * @brief Creates a predictor filter.
 *
 * @param predictor The @c /Predictor value: 2 (TIFF) or 10-15 (PNG).
 * @param colors The number of interleaved color components (@c /Colors).
 * @param bits_per_component The bit depth of each component (@c /BitsPerComponent).
 * @param columns The number of samples per row (@c /Columns).
 * @param out_error On failure, set to describe the error.
 *
 * @return The new predictor filter, or @c NULL if the parameters are unsupported
 *   (@c COS_ERROR_NOT_IMPLEMENTED), invalid (@c COS_ERROR_SYNTAX), or allocation
 *   failed (@c COS_ERROR_MEMORY).
 */
COS_API CosPredictorFilter * COS_Nullable
cos_predictor_filter_create_(int predictor,
                             size_t colors,
                             size_t bits_per_component,
                             size_t columns,
                             CosError * COS_Nullable out_error)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_stream_close)
    COS_ATTR_ACCESS_WRITE_ONLY(5);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_FILTERS_COS_PREDICTOR_FILTER_H */
