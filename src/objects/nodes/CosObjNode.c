/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/objects/nodes/CosObjNode.h"

#include "common/Assert.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

struct CosObjNode {
    CosObjNodeType type;
};

CosObjNodeType
cos_obj_node_get_type(const CosObjNode *node)
{
    COS_PARAMETER_ASSERT(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return CosObjNodeType_Unknown;
    }

    return node->type;
}

COS_ASSUME_NONNULL_END
