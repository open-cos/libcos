/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_BASIC_TYPES_H
#define LIBCOS_COMMON_COS_BASIC_TYPES_H

#include <libcos/common/CosDefines.h>

#include <stdint.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief The object number of an indirect object.
 */
typedef uint32_t CosObjNumber;

/**
 * @brief The generation number of an indirect object.
 *
 * The maximum generation number is 65535; when this value is reached, the corresponding object
 * number shall never be reused.
 */
typedef uint16_t CosObjGenNumber;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_BASIC_TYPES_H */
