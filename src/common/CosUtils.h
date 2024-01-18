/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_UTILS_H
#define LIBCOS_COMMON_COS_UTILS_H

#include <libcos/common/CosDefines.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

#pragma mark - Bit Operations

/**
 * @brief Returns the index of the last set bit.
 *
 * @param mask The mask to search.
 *
 * @return The index of the last set bit, or @c 0 if no bits are set in @p mask.
 */
int
cos_fls(int mask)
    COS_ATTR_PURE;

/**
 * @brief Returns the index of the last set bit.
 *
 * @param mask The mask to search.
 *
 * @return The index of the last set bit, or @c 0 if no bits are set in @p mask.
 */
int
cos_flsl(long mask)
    COS_ATTR_PURE;

/**
 * @brief Returns the index of the last set bit.
 *
 * @param mask The mask to search.
 *
 * @return The index of the last set bit, or @c 0 if no bits are set in @p mask.
 */
int
cos_flsll(long long mask)
    COS_ATTR_PURE;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_UTILS_H */
