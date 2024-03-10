/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_NUMBER_H
#define LIBCOS_COMMON_COS_NUMBER_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef enum CosNumberType {
    CosNumberType_Integer,
    CosNumberType_LongInteger,
    CosNumberType_Real,
} CosNumberType;

struct CosNumber {
    CosNumberType type;

    union {
        int integer;
        long long int long_integer;
        double real;
    } value;
};

static inline CosNumber
cos_number_make_integer(int value)
{
    const CosNumber number_value = {
        .type = CosNumberType_Integer,
        .value.integer = value,
    };
    return number_value;
}

static inline CosNumber
cos_number_make_long_integer(long long int value)
{
    const CosNumber number_value = {
        .type = CosNumberType_LongInteger,
        .value.long_integer = value,
    };
    return number_value;
}

static inline CosNumber
cos_number_make_real(double value)
{
    const CosNumber number_value = {
        .type = CosNumberType_Real,
        .value.real = value,
    };
    return number_value;
}

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_NUMBER_H */
