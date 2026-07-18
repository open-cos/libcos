/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_CONTAINER_UTILS_H
#define LIBCOS_COMMON_COS_CONTAINER_UTILS_H

#include "common/CosUtils.h"

#include <libcos/common/CosDefines.h>

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief Rounds up the capacity to the next power of 2.
 *
 * Saturates at @c SIZE_MAX when no larger power of two is representable. The
 * result is therefore always at least @p capacity, which is what every caller
 * relies on: a container that rounded a large request *down* would report the
 * resize as successful and then be written past the end of its buffer.
 *
 * @param capacity The capacity to be rounded.
 *
 * @return The rounded capacity, never less than @p capacity.
 */
COS_STATIC_INLINE size_t
cos_container_round_capacity_(size_t capacity)
{
    // The largest power of two a size_t can hold.
    const size_t highest_pow2 = ((size_t)1) << ((sizeof(size_t) * CHAR_BIT) - 1);

    if (capacity < 4) {
        return 4;
    }

    /*
     * Above this there is no next power of two to round to. The rounding
     * helpers compute 1 << fls(x), which is undefined once the shift reaches
     * the width of the type -- it used to return 1 here, so a request for
     * SIZE_MAX-1 elements produced a one-element buffer and a successful
     * resize. Saturating instead leaves the caller's allocation to fail.
     */
    if (capacity >= highest_pow2 - 1) {
        return SIZE_MAX;
    }

    // Round up to the next power of 2.
#if SIZE_MAX == UINT32_MAX
    return cos_next_pow2l(capacity + 1);
#elif SIZE_MAX == UINT64_MAX
    return cos_next_pow2ll(capacity + 1);
#else
    return cos_next_pow2(capacity + 1);
#endif
}

COS_DECLS_END
COS_ASSUME_NONNULL_END

#endif /* LIBCOS_COMMON_COS_CONTAINER_UTILS_H */
