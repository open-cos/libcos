/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_NODES_COS_REAL_NODE_H
#define LIBCOS_NODES_COS_REAL_NODE_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

COS_API CosRealNode * COS_Nullable
cos_real_node_create(CosAllocator *allocator,
                      double value)
    COS_ALLOCATOR_FUNC;

/**
 * Returns the value of the real node.
 *
 * @param node The real node.
 *
 * @return The value of the real node.
 */
COS_API double
cos_real_node_get_value(const CosRealNode *node)
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_NODES_COS_REAL_NODE_H */
