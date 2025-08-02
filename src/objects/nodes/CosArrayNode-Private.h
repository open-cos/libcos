/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_COS_ARRAY_NODE_PRIVATE_H
#define LIBCOS_COS_ARRAY_NODE_PRIVATE_H

#include <libcos/common/CosDefines.h>
#include <libcos/objects/nodes/CosArrayNode.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

COS_API void
cos_array_node_deinit_(CosArrayNode *array_node);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_ARRAY_NODE_PRIVATE_H */
