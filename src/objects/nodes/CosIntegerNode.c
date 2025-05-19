/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/objects/nodes/CosIntegerNode.h"

#include "CosNode-Private.h"
#include "common/Assert.h"

#include "libcos/common/memory/CosAllocator.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

struct CosIntegerNode {
    CosNode base;

    int value;
};

static bool
cos_integer_node_init_(CosIntegerNode *node,
                       int value)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return false;
    }

    if (!cos_node_init_(&node->base,
                        CosNodeType_Integer)) {
        return false;
    }

    node->value = value;

    return true;
}

CosIntegerNode *
cos_integer_node_create(CosAllocator *allocator,
                        int value)
{
    COS_API_PARAM_CHECK(allocator != NULL);
    if (COS_UNLIKELY(!allocator)) {
        return NULL;
    }

    CosIntegerNode *node = cos_allocator_alloc(allocator,
                                               sizeof(CosIntegerNode));
    if (COS_UNLIKELY(!node)) {
        goto failure;
    }

    if (!cos_integer_node_init_(node,
                                value)) {
        goto failure;
    }

    return node;

failure:
    if (node) {
        cos_allocator_dealloc(allocator, node);
    }
    return NULL;
}

int
cos_integer_node_get_value(const CosIntegerNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return 0;
    }

    return node->value;
}

COS_ASSUME_NONNULL_END
