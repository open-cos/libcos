/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "common/CosUtils.h"

#include <limits.h>
#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

int
cos_fls(int mask)
{
    // The value of __builtin_fls(0) and __builtin_clz(0) is undefined.
    if (mask == 0) {
        return 0;
    }
#if COS_HAS_BUILTIN(__builtin_fls)
    return __builtin_fls(mask);
#elif COS_HAS_BUILTIN(__builtin_clz)
    const size_t total_bits = sizeof(mask) * CHAR_BIT;
    const int leading_zeros = __builtin_clz((unsigned int)mask);
    return (int)(total_bits - (size_t)leading_zeros);
#else
    int bits = 0;
    // Avoid sign extension by shifting with an unsigned type.
    unsigned int umask = (unsigned int)mask;
    while (umask != 0) {
        umask = umask >> 1;
        bits++;
    }
    return bits;
#endif
}

int
cos_flsl(long mask)
{
    // The value of __builtin_flsl(0) and __builtin_clzl(0) is undefined.
    if (mask == 0) {
        return 0;
    }
#if COS_HAS_BUILTIN(__builtin_flsl)
    return __builtin_flsl(mask);
#elif COS_HAS_BUILTIN(__builtin_clzl)
    const size_t total_bits = sizeof(mask) * CHAR_BIT;
    const int leading_zeros = __builtin_clzl((unsigned long)mask);
    return (int)(total_bits - (size_t)leading_zeros);
#else
    int bits = 0;
    // Avoid sign extension by shifting with an unsigned type.
    unsigned long umask = (unsigned long)mask;
    while (umask != 0) {
        umask = umask >> 1;
        bits++;
    }
    return bits;
#endif
}

int
cos_flsll(long long mask)
{
    // The value of __builtin_flsll(0) and __builtin_clzll(0) is undefined.
    if (mask == 0) {
        return 0;
    }
#if COS_HAS_BUILTIN(__builtin_flsll)
    return __builtin_flsll(mask);
#elif COS_HAS_BUILTIN(__builtin_clzll)
    const size_t total_bits = sizeof(mask) * CHAR_BIT;
    const int leading_zeros = __builtin_clzll((unsigned long long)mask);
    return (int)(total_bits - (size_t)leading_zeros);
#else
    int bits = 0;
    // Avoid sign extension by shifting with an unsigned type.
    unsigned long long umask = (unsigned long long)mask;
    while (umask != 0) {
        umask = umask >> 1;
        bits++;
    }
    return bits;
#endif
}

COS_ASSUME_NONNULL_END
