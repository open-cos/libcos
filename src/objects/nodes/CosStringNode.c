/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/objects/nodes/CosStringNode.h"

#include "CosNode-Private.h"
#include "common/Assert.h"
#include "common/memory/CosMemory.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

struct CosStringNode {
    CosNode base;

    CosData * COS_Nullable value;
};

static bool
cos_string_node_init_(CosStringNode *string_node,
                      CosData *value)
    COS_WARN_UNUSED_RESULT;

CosStringNode *
cos_string_node_create(CosAllocator *allocator,
                       CosData *value)
{
    COS_API_PARAM_CHECK(allocator != NULL);
    COS_API_PARAM_CHECK(value != NULL);
    if (COS_UNLIKELY(!allocator || !value)) {
        return NULL;
    }

    CosStringNode * const string_node = cos_alloc(allocator,
                                                  sizeof(CosStringNode));
    if (COS_UNLIKELY(!string_node)) {
        goto failure;
    }

    if (!cos_string_node_init_(string_node,
                               value)) {
        goto failure;
    }

    return string_node;

failure:
    if (string_node) {
        cos_free(allocator, string_node);
    }
    return NULL;
}

CosData *
cos_string_node_get_value(const CosStringNode *string_node)
{
    COS_API_PARAM_CHECK(string_node != NULL);
    if (COS_UNLIKELY(!string_node)) {
        return NULL;
    }

    return string_node->value;
}

static bool
cos_string_node_init_(CosStringNode *string_node,
                      CosData *value)
{
    COS_API_PARAM_CHECK(string_node != NULL);
    if (COS_UNLIKELY(!string_node)) {
        return false;
    }

    if (!cos_node_init(&string_node->base,
                       CosNodeType_String)) {
        return false;
    }

    string_node->value = value;

    return true;
}

COS_ASSUME_NONNULL_END
