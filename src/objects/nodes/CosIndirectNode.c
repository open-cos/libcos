/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/objects/nodes/CosIndirectNode.h"

#include "CosNode-Private.h"
#include "common/Assert.h"

#include "libcos/common/memory/CosMemory.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

struct CosIndirectNode {
    CosNode base;

    CosObjID id;

    CosNode * COS_Nullable value;
};

static bool
cos_indirect_node_init_(CosIndirectNode *node,
                        CosObjID id,
                        CosNode * COS_Nullable value)
{
    COS_API_PARAM_CHECK(node != NULL);
    COS_API_PARAM_CHECK(cos_obj_id_is_valid(id));
    if (COS_UNLIKELY(!node || !cos_obj_id_is_valid(id))) {
        return false;
    }

    if (!cos_node_init_(&node->base,
                        CosNodeType_Indirect)) {
        return false;
    }

    node->id = id;
    node->value = value;

    return true;
}

CosIndirectNode *
cos_indirect_node_create(CosAllocator *allocator,
                         CosObjID id,
                         CosNode * COS_Nullable value)
{
    COS_API_PARAM_CHECK(allocator != NULL);
    COS_API_PARAM_CHECK(cos_obj_id_is_valid(id));
    if (COS_UNLIKELY(!allocator || !cos_obj_id_is_valid(id))) {
        return NULL;
    }

    CosIndirectNode *node = cos_alloc(allocator,
                                      sizeof(CosIndirectNode));
    if (COS_UNLIKELY(!node)) {
        goto failure;
    }

    if (!cos_indirect_node_init_(node,
                                 id,
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

CosObjID
cos_indirect_node_get_id(const CosIndirectNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (!node) {
        return CosObjID_Invalid;
    }

    return node->id;
}

CosNode *
cos_indirect_node_get_value(const CosIndirectNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (!node) {
        return NULL;
    }

    return node->value;
}

COS_ASSUME_NONNULL_END
