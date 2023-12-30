/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/CosNullObj.h"

COS_ASSUME_NONNULL_BEGIN

static CosNullObj cos_null_obj_ = {
    .base = {
        .document = NULL,
    },
};

CosNullObj *
cos_null_obj_get(void)
{
    return &cos_null_obj_;
}

COS_ASSUME_NONNULL_END
