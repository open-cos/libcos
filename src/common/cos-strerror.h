/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_STRERROR_H
#define LIBCOS_COMMON_COS_STRERROR_H

#include <libcos/common/CosDefines.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief Returns a string describing the error number.
 *
 * @param errnum The error number.
 *
 * @return The error string, or @c NULL if an error occurred.
 */
char * COS_Nullable
cos_strerror(int errnum)
    COS_OWNERSHIP_RETURNS
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(free);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_STRERROR_H */
