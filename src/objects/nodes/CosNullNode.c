/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/objects/nodes/CosNullNode.h"

#include "CosNode-Private.h"
#include "common/Assert.h"

#include "libcos/common/memory/CosMemory.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

struct CosNullNode {
    CosNode base;

    /* No additional data. */
};

static bool
cos_null_node_init_(CosNullNode *node)
{
    COS_IMPL_PARAM_CHECK(node != NULL);

    if (!cos_node_init_(&node->base,
                        CosNodeType_Null)) {
        return false;
    }

    return true;
}

CosNullNode *
cos_null_node_create(CosAllocator *allocator)
{
    COS_API_PARAM_CHECK(allocator != NULL);
    if (COS_UNLIKELY(!allocator)) {
        return NULL;
    }

    CosNullNode *node = cos_alloc(allocator,
                                  sizeof(CosNullNode));
    if (COS_UNLIKELY(!node)) {
        goto failure;
    }

    if (!cos_null_node_init_(node)) {
        goto failure;
    }

    return node;

failure:
    if (node) {
        cos_free(allocator,
                 node);
    }

    return NULL;
}

COS_ASSUME_NONNULL_END
