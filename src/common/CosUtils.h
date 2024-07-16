/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_UTILS_H
#define LIBCOS_COMMON_COS_UTILS_H

#include <libcos/common/CosDefines.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

// MARK: - Math

/**
 * @brief Returns the next power of 2 greater than or equal to @p x.
 *
 * @param x The value to be rounded.
 *
 * @return The next power of 2 greater than or equal to @p x.
 */
unsigned int
cos_next_pow2(unsigned int x)
    COS_ATTR_PURE;

/**
 * @brief Returns the next power of 2 greater than or equal to @p x.
 *
 * @param x The value to be rounded.
 *
 * @return The next power of 2 greater than or equal to @p x.
 */
unsigned long
cos_next_pow2l(unsigned long x)
    COS_ATTR_PURE;

/**
 * @brief Returns the next power of 2 greater than or equal to @p x.
 *
 * @param x The value to be rounded.
 *
 * @return The next power of 2 greater than or equal to @p x.
 */
unsigned long long
cos_next_pow2ll(unsigned long long x)
    COS_ATTR_PURE;

// MARK: - Bit Operations

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
