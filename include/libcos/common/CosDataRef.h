/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_DATA_REF_H
#define LIBCOS_COMMON_COS_DATA_REF_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosDataRef {
    const unsigned char * COS_Nullable bytes;
    size_t size;
};

/**
 * @brief Creates a data reference.
 *
 * @param bytes The bytes to reference.
 * @param size The number of bytes.
 *
 * @return A data reference.
 */
CosDataRef
cos_data_ref_make(const unsigned char * COS_Nullable bytes,
                  size_t size);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_DATA_REF_H */
