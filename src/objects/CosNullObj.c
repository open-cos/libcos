/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/objects/CosNullObj.h"

#include "common/Assert.h"
#include "libcos/objects/CosObj.h"

#include <stdio.h>

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

void
cos_null_obj_print_desc(const CosNullObj *null_obj)
{
    COS_PARAMETER_ASSERT(null_obj != NULL);
    if (!null_obj) {
        return;
    }

    printf("null");
}

COS_ASSUME_NONNULL_END
