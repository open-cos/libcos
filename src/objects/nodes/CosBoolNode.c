/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/objects/nodes/CosBoolNode.h"

#include "CosNode-Private.h"
#include "common/Assert.h"

#include "libcos/common/memory/CosAllocator.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

struct CosBoolNode {
    CosNode base;

    bool value;
};

static bool
cos_bool_node_init(CosBoolNode *bool_node,
                   bool value)
    COS_WARN_UNUSED_RESULT;

CosBoolNode *
cos_bool_node_create(CosAllocator *allocator,
                     bool value)
{
    COS_API_PARAM_CHECK(allocator != NULL);
    if (COS_UNLIKELY(!allocator)) {
        return NULL;
    }

    CosBoolNode *bool_node = cos_allocator_alloc(allocator,
                                                 sizeof(CosBoolNode));
    if (COS_UNLIKELY(!bool_node)) {
        goto failure;
    }

    if (!cos_bool_node_init(bool_node,
                            value)) {
        goto failure;
    }

    return bool_node;

failure:
    if (bool_node) {
        cos_allocator_dealloc(allocator, bool_node);
    }
    return NULL;
}

bool
cos_bool_node_get_value(const CosBoolNode *bool_node)
{
    COS_API_PARAM_CHECK(bool_node != NULL);
    if (COS_UNLIKELY(!bool_node)) {
        return false;
    }

    return bool_node->value;
}

static bool
cos_bool_node_init(CosBoolNode *bool_node,
                   bool value)
{
    COS_API_PARAM_CHECK(bool_node != NULL);
    if (COS_UNLIKELY(!bool_node)) {
        return false;
    }

    if (!cos_node_init(&bool_node->base,
                       CosNodeType_Boolean)) {
        return false;
    }

    bool_node->value = value;

    return true;
}

COS_ASSUME_NONNULL_END
