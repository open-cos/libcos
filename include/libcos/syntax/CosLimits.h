/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_SYNTAX_COS_LIMITS_H
#define LIBCOS_SYNTAX_COS_LIMITS_H

#include <libcos/common/CosDefines.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief The largest integer value.
 */
#define COS_INT_MAX 2147483647

/**
 * @brief The smallest integer value.
 */
#define COS_INT_MIN (-2147483648)

#define COS_REAL_MAX 3.403E+38

/**
 * @brief The smallest positive real value.
 */
#define COS_REAL_MIN 1.175E-38

#define COS_REAL_MAX_SIG_FRAC_DIG 5

#define COS_STRING_MAX_LENGTH 32767

/**
 * @brief The maximum length of a name, in bytes.
 */
#define COS_NAME_MAX_LENGTH 127

#define COS_INDIRECT_OBJ_MAX_NUMBER 8388607

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_SYNTAX_COS_LIMITS_H */
