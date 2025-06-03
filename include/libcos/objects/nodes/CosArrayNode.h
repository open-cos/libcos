/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_NODES_COS_ARRAY_NODE_H
#define LIBCOS_NODES_COS_ARRAY_NODE_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

COS_API CosArrayNode * COS_Nullable
cos_array_node_create(CosAllocator *allocator,
                      CosArray *value)
    COS_ALLOCATOR_FUNC
    COS_OWNERSHIP_HOLDS(2);

/**
 * Returns the value of the array node.
 *
 * @param array_node The array node.
 *
 * @return The value of the array node.
 */
COS_API CosArray * COS_Nullable
cos_array_node_get_value(const CosArrayNode *array_node)
    COS_WARN_UNUSED_RESULT;

COS_API CosNode * COS_Nullable
cos_array_node_get_child(CosArrayNode *array_node,
                         size_t index,
                         CosError * COS_Nullable out_error)
    COS_WARN_UNUSED_RESULT
    COS_ATTR_ACCESS_WRITE_ONLY(3);

COS_ASSUME_NONNULL_END

#endif /* LIBCOS_NODES_COS_ARRAY_NODE_H */
