/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/objects/nodes/CosDictNode.h"

#include "CosNode-Private.h"
#include "common/Assert.h"
#include "common/memory/CosAllocator.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

struct CosDictNode {
    CosNode base;

    CosDict * COS_Nullable value;
};

static bool
cos_dict_node_init(CosDictNode *dict_node,
                   CosDict *value)
    COS_WARN_UNUSED_RESULT
    COS_OWNERSHIP_HOLDS(2);

CosDict *
cos_dict_node_get_value(const CosDictNode *dict_node)
{
    COS_API_PARAM_CHECK(dict_node != NULL);
    if (COS_UNLIKELY(!dict_node)) {
        return NULL;
    }

    return dict_node->value;
}

CosDictNode *
cos_dict_node_create(CosAllocator *allocator,
                     CosDict *value)
{
    COS_API_PARAM_CHECK(allocator != NULL);
    COS_API_PARAM_CHECK(value != NULL);
    if (COS_UNLIKELY(!allocator || !value)) {
        return NULL;
    }

    CosDictNode *dict_node = cos_allocator_alloc(allocator,
                                                 sizeof(CosDictNode));
    if (COS_UNLIKELY(!dict_node)) {
        goto failure;
    }

    if (!cos_dict_node_init(dict_node,
                            value)) {
        goto failure;
    }

    return dict_node;

failure:
    if (dict_node) {
        cos_allocator_dealloc(allocator, dict_node);
    }
    return NULL;
}

static bool
cos_dict_node_init(CosDictNode *dict_node,
                   CosDict *value)
{
    COS_API_PARAM_CHECK(dict_node != NULL);
    if (COS_UNLIKELY(!dict_node)) {
        return false;
    }

    if (!cos_node_init(&dict_node->base,
                       CosNodeType_Dictionary)) {
        return false;
    }

    dict_node->value = value;

    return true;
}

COS_ASSUME_NONNULL_END
