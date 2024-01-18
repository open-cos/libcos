/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/objects/CosNullObj.h"

#include "libcos/objects/CosObj.h"

COS_ASSUME_NONNULL_BEGIN

struct CosNullObj {
    CosObjType type;

    void * COS_Nullable data;
};

static CosNullObj cos_null_obj_ = {
    .type = CosObjType_Null,
    .data = NULL,
};

CosNullObj *
cos_null_obj_get(void)
{
    return &cos_null_obj_;
}

COS_ASSUME_NONNULL_END
