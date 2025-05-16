/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/objects/nodes/CosRealNode.h"

#include "CosNode-Private.h"
#include "common/Assert.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

struct CosRealNode {
    CosNode base;

    double value;
};

double
cos_real_node_get_value(const CosRealNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return 0.0;
    }

    return node->value;
}

COS_ASSUME_NONNULL_END
