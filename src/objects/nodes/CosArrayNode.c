/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/objects/nodes/CosArrayNode.h"

#include "CosNode-Private.h"
#include "common/Assert.h"

#include "libcos/common/memory/CosAllocator.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

struct CosArrayNode {
    CosNode base;

    CosArray * COS_Nullable value;
};

static bool
cos_array_node_init_(CosArrayNode *array_node,
                     CosArray *value)
{
    COS_API_PARAM_CHECK(array_node != NULL);
    if (COS_UNLIKELY(!array_node)) {
        return false;
    }

    if (!cos_node_init_(&array_node->base,
                        CosNodeType_Array)) {
        return false;
    }

    array_node->value = value;

    return true;
}

CosArrayNode *
cos_array_node_create(CosAllocator *allocator,
                      CosArray *value)
{
    COS_API_PARAM_CHECK(allocator != NULL);
    COS_API_PARAM_CHECK(value != NULL);
    if (COS_UNLIKELY(!allocator || !value)) {
        return NULL;
    }

    CosArrayNode *array_node = cos_allocator_alloc(allocator,
                                                   sizeof(CosArrayNode));
    if (COS_UNLIKELY(!array_node)) {
        goto failure;
    }

    if (!cos_array_node_init_(array_node,
                              value)) {
        goto failure;
    }

    return array_node;

failure:
    if (array_node) {
        cos_allocator_dealloc(allocator, array_node);
    }
    return NULL;
}

CosArray *
cos_array_node_get_value(const CosArrayNode *array_node)
{
    COS_API_PARAM_CHECK(array_node != NULL);
    if (COS_UNLIKELY(!array_node)) {
        return NULL;
    }

    return array_node->value;
}

COS_ASSUME_NONNULL_END
