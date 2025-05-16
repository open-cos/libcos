/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_NODES_COS_NAME_NODE_H
#define LIBCOS_NODES_COS_NAME_NODE_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * Returns the value of the name node.
 *
 * @param name_node The name node.
 *
 * @return The value of the name node.
 */
COS_API CosString * COS_Nullable
cos_name_node_get_value(const CosNameNode *name_node)
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_NODES_COS_NAME_NODE_H */
