/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/objects/nodes/CosStreamNode.h"

#include "CosNode-Private.h"
#include "common/Assert.h"
#include "common/memory/CosAllocator.h"
#include "common/memory/CosMemory.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

struct CosStreamNode {
    CosNode base;

    CosDictNode *dict;

    CosData *data;
};

static bool
cos_stream_node_init_(CosStreamNode *node,
                      CosDictNode *dict,
                      CosData *data)
    COS_WARN_UNUSED_RESULT;

CosStreamNode *
cos_stream_node_create(CosAllocator *allocator,
                       CosDictNode *dict,
                       CosData *data)
{
    COS_API_PARAM_CHECK(allocator != NULL);
    COS_API_PARAM_CHECK(dict != NULL);
    if (COS_UNLIKELY(!allocator || !dict)) {
        return NULL;
    }

    CosStreamNode * const node = cos_alloc(allocator,
                                           sizeof(CosStreamNode));

    if (COS_UNLIKELY(!node)) {
        goto failure;
    }

    if (!cos_stream_node_init_(node,
                               dict,
                               data)) {
        goto failure;
    }

    return node;

failure:
    if (node) {
        cos_free(allocator, node);
    }
    return NULL;
}

CosDictNode *
cos_stream_node_get_dict(const CosStreamNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return NULL;
    }

    return node->dict;
}

static bool
cos_stream_node_init_(CosStreamNode *node,
                      CosDictNode *dict,
                      CosData *data)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return false;
    }

    if (!cos_node_init(&node->base,
                       CosNodeType_Stream)) {
        return false;
    }

    node->dict = dict;
    node->data = data;

    return true;
}

COS_ASSUME_NONNULL_END
