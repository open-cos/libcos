/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/objects/CosNullObj.h"

#include "libcos/private/objects/CosNullObj-Impl.h"

COS_ASSUME_NONNULL_BEGIN

static CosNullObj cos_null_obj_ = {
    .base = {
        .type = CosObjectType_Null,
    },
};

CosNullObj *
cos_null_obj_get(void)
{
    return &cos_null_obj_;
}

COS_ASSUME_NONNULL_END
