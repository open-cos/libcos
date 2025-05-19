/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/objects/nodes/CosNameNode.h"

#include "CosNode-Private.h"
#include "common/Assert.h"

#include "libcos/common/memory/CosMemory.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

struct CosNameNode {
    CosNode base;

    CosString * COS_Nullable value;
};

static bool
cos_name_node_init_(CosNameNode *name_node,
                    CosString *value)
{
    COS_IMPL_PARAM_CHECK(name_node != NULL);
    COS_IMPL_PARAM_CHECK(value != NULL);

    if (!cos_node_init_(&name_node->base,
                        CosNodeType_Name)) {
        return false;
    }

    name_node->value = value;

    return true;
}

CosNameNode *
cos_name_node_create(CosAllocator *allocator,
                     CosString *value)
{
    COS_API_PARAM_CHECK(allocator != NULL);
    COS_API_PARAM_CHECK(value != NULL);
    if (COS_UNLIKELY(!allocator || !value)) {
        return NULL;
    }

    CosNameNode *name_node = cos_alloc(allocator,
                                       sizeof(CosNameNode));
    if (COS_UNLIKELY(!name_node)) {
        goto failure;
    }

    if (!cos_name_node_init_(name_node,
                             value)) {
        goto failure;
    }

    return name_node;

failure:
    if (name_node) {
        cos_free(allocator, name_node);
    }
    return NULL;
}

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
