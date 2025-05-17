/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/objects/nodes/CosRealNode.h"

#include "CosNode-Private.h"
#include "common/Assert.h"
#include "common/memory/CosMemory.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

struct CosRealNode {
    CosNode base;

    double value;
};

static bool
cos_real_node_init_(CosRealNode *node,
                    double value)
{
    COS_IMPL_PARAM_CHECK(node != NULL);

    if (!cos_node_init(&node->base,
                       CosNodeType_Real)) {
        return false;
    }

    node->value = value;

    return true;
}

CosRealNode *
cos_real_node_create(CosAllocator *allocator,
                     double value)
{
    COS_API_PARAM_CHECK(allocator != NULL);
    if (COS_UNLIKELY(!allocator)) {
        return NULL;
    }

    CosRealNode *node = cos_alloc(allocator,
                                  sizeof(CosRealNode));
    if (COS_UNLIKELY(!node)) {
        goto failure;
    }

    if (!cos_real_node_init_(node,
                             value)) {
        goto failure;
    }

    return node;

failure:
    if (node) {
        cos_free(allocator, node);
    }
    return NULL;
}

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
