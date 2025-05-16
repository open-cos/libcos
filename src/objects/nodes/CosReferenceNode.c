/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/objects/nodes/CosReferenceNode.h"

#include "CosNode-Private.h"
#include "common/Assert.h"

#include "libcos/common/memory/CosMemory.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

struct CosReferenceNode {
    CosNode base;

    CosObjID id;
};

static bool
cos_reference_node_init_(CosReferenceNode *node,
                         CosObjID id)
    COS_WARN_UNUSED_RESULT;

CosReferenceNode *
cos_reference_node_create(CosAllocator *allocator,
                          CosObjID id)
{
    COS_API_PARAM_CHECK(allocator != NULL);
    COS_API_PARAM_CHECK(cos_obj_id_is_valid(id));
    if (COS_UNLIKELY(!allocator || !cos_obj_id_is_valid(id))) {
        return NULL;
    }

    CosReferenceNode *node = cos_alloc(allocator,
                                       sizeof(CosReferenceNode));
    if (COS_UNLIKELY(!node)) {
        goto failure;
    }

    if (!cos_reference_node_init_(node,
                                  id)) {
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
cos_reference_node_get_id(const CosReferenceNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (!node) {
        return CosObjID_Invalid;
    }

    return node->id;
}

static bool
cos_reference_node_init_(CosReferenceNode *node,
                         CosObjID id)
{
    COS_API_PARAM_CHECK(node != NULL);
    COS_API_PARAM_CHECK(cos_obj_id_is_valid(id));
    if (COS_UNLIKELY(!node || cos_obj_id_is_valid(id))) {
        return false;
    }

    if (!cos_node_init(&node->base,
                       CosNodeType_Reference)) {
        return false;
    }

    node->id = id;

    return true;
}

COS_ASSUME_NONNULL_END
