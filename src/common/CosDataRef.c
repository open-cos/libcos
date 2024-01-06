/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/common/CosDataRef.h"

COS_ASSUME_NONNULL_BEGIN

CosDataRef
cos_data_ref_make(const unsigned char * COS_Nullable bytes,
                  size_t size)
{
    return (CosDataRef){
        .bytes = bytes,
        .size = size,
    };
}

COS_ASSUME_NONNULL_END
