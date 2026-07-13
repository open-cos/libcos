/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_FILTERS_COS_FILTER_FACTORY_H
#define LIBCOS_FILTERS_COS_FILTER_FACTORY_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * Creates a decode filter for a PDF stream filter name.
 *
 * @param name The filter name, e.g. @c "FlateDecode" or its abbreviation @c "Fl".
 * @param out_error On failure, set to describe the error.
 *
 * @return A new filter stream (owned; close with @c cos_stream_close), or @c NULL
 *   if the name is unknown/unsupported (@c COS_ERROR_NOT_IMPLEMENTED) or allocation
 *   failed (@c COS_ERROR_MEMORY).
 */
CosStream * COS_Nullable
cos_filter_create_for_name_(const CosString *name,
                            CosError * COS_Nullable out_error)
    COS_OWNERSHIP_RETURNS
    COS_ATTR_ACCESS_WRITE_ONLY(2);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_FILTERS_COS_FILTER_FACTORY_H */
