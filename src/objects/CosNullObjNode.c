/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/objects/CosNullObjNode.h"

#include "common/Assert.h"

#include "libcos/objects/CosObjNode.h"

#include <stdio.h>

COS_ASSUME_NONNULL_BEGIN

struct CosNullObjNode {
    CosObjNodeType type;

    void * COS_Nullable data;
};

static CosNullObjNode cos_null_obj_node_ = {
    .type = CosObjNodeType_Null,
    .data = NULL,
};

CosNullObjNode *
cos_null_obj_node_get(void)
{
    return &cos_null_obj_node_;
}

void
cos_null_obj_node_print_desc(const CosNullObjNode *null_obj)
{
    COS_API_PARAM_CHECK(null_obj != NULL);
    if (!null_obj) {
        return;
    }

    printf("null");
}

COS_ASSUME_NONNULL_END
