/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/objects/nodes/CosNameNode.h"

#include "CosNode-Private.h"
#include "common/Assert.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

struct CosNameNode {
    CosNode base;

    CosString * COS_Nullable value;
};

CosString *
cos_name_node_get_value(const CosNameNode *name_node)
{
    COS_API_PARAM_CHECK(name_node != NULL);
    if (COS_UNLIKELY(!name_node)) {
        return NULL;
    }

    return name_node->value;
}

COS_ASSUME_NONNULL_END
