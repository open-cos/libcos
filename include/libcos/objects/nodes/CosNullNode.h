/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_NODES_COS_NULL_NODE_H
#define LIBCOS_NODES_COS_NULL_NODE_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

COS_API CosNullNode * COS_Nullable
cos_null_node_create(CosAllocator *allocator)
    COS_ALLOCATOR_FUNC;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_NODES_COS_NULL_NODE_H */
