/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_CONTAINER_UTILS_H
#define LIBCOS_COMMON_COS_CONTAINER_UTILS_H

#include "common/CosUtils.h"

#include <libcos/common/CosDefines.h>

#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief Rounds up the capacity to the next power of 2.
 *
 * @param capacity The capacity to be rounded.
 *
 * @return The rounded capacity.
 */
COS_STATIC_INLINE size_t
cos_container_round_capacity_(size_t capacity)
{
    if (capacity < 4) {
        return 4;
    }
    // Round up to the next power of 2.
    return cos_next_pow2l(capacity + 1);
}

COS_DECLS_END
COS_ASSUME_NONNULL_END

#endif /* LIBCOS_COMMON_COS_CONTAINER_UTILS_H */
