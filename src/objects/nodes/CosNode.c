/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/objects/nodes/CosNode.h"

#include "common/Assert.h"
#include "objects/nodes/CosArrayNode-Private.h"
#include "objects/nodes/CosNode-Private.h"

#include "libcos/common/memory/CosMemory.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

bool
cos_node_init_(CosNode *node,
               CosNodeType type)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return false;
    }

    node->type = type;
    node->parent = NULL;

    return true;
}

void
cos_node_destroy(CosNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return;
    }

    switch (node->type) {
        case CosNodeType_Unknown:
            break;
        case CosNodeType_Array: {
            cos_array_node_deinit_((CosArrayNode *)node);
        } break;
        case CosNodeType_Dictionary:
            break;
        case CosNodeType_Stream:
            break;
        case CosNodeType_Indirect:
            break;
        case CosNodeType_Reference:
            break;
        case CosNodeType_Null:
            break;
        case CosNodeType_Boolean:
            break;
        case CosNodeType_Integer:
            break;
        case CosNodeType_Real:
            break;
        case CosNodeType_String:
            break;
        case CosNodeType_Name:
            break;
    }

    // Free the node itself.
    cos_free(node->allocator, node);
}

CosNodeType
cos_obj_node_get_type(const CosNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return CosNodeType_Unknown;
    }

    return node->type;
}

CosNode *
cos_node_get_parent(const CosNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return NULL;
    }

    return node->parent;
}

// MARK: - Reference counting

void
cos_node_retain(CosNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return;
    }

    node->retain_count++;
}

void
cos_node_release(CosNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return;
    }

    // Make sure that we are not over-releasing the node.
    if (COS_UNLIKELY(node->retain_count == 0)) {
        // TODO: Log warning for over-released node.
        return;
    }

    node->retain_count--;

    if (COS_UNLIKELY(node->retain_count == 0)) {
        cos_node_destroy(node);
    }
}

// MARK: - Type checkers

bool
cos_node_is_bool(const CosNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return false;
    }

    return (node->type == CosNodeType_Boolean);
}

bool
cos_node_is_number(const CosNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return false;
    }

    return (node->type == CosNodeType_Integer ||
            node->type == CosNodeType_Real);
}

bool
cos_node_is_integer(const CosNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return false;
    }

    return (node->type == CosNodeType_Integer);
}

bool
cos_node_is_real(const CosNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return false;
    }

    return (node->type == CosNodeType_Real);
}

bool
cos_node_is_string(const CosNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return false;
    }

    return (node->type == CosNodeType_String);
}

bool
cos_node_is_name(const CosNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return false;
    }

    return (node->type == CosNodeType_Name);
}

bool
cos_node_is_array(const CosNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return false;
    }

    return (node->type == CosNodeType_Array);
}

bool
cos_node_is_dict(const CosNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return false;
    }

    return (node->type == CosNodeType_Dictionary);
}

bool
cos_node_is_stream(const CosNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return false;
    }

    return (node->type == CosNodeType_Stream);
}

bool
cos_node_is_indirect(const CosNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return false;
    }

    return (node->type == CosNodeType_Indirect);
}

bool
cos_node_is_reference(const CosNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return false;
    }

    return (node->type == CosNodeType_Reference);
}

bool
cos_node_is_null(const CosNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return false;
    }

    return (node->type == CosNodeType_Null);
}

COS_ASSUME_NONNULL_END
