/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_NODES_COS_NODE_PRIVATE_H
#define LIBCOS_NODES_COS_NODE_PRIVATE_H

#include <libcos/common/CosDefines.h>
#include <libcos/objects/nodes/CosNode.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosNode {
    CosNodeType type;
    CosNode * COS_Nullable parent;
};

/**
 * Initializes a node.
 *
 * @param node The node to initialize.
 * @param type The type of the node.
 *
 * @return @c true if the node was initialized successfully, @c false otherwise.
 */
COS_API bool
cos_node_init_(CosNode *node,
              CosNodeType type)
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_NODES_COS_NODE_PRIVATE_H */
