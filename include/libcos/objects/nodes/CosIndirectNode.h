/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_OBJECTS_NODES_COS_INDIRECT_NODE_H
#define LIBCOS_OBJECTS_NODES_COS_INDIRECT_NODE_H

#include <libcos/CosObjID.h>
#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

COS_API CosIndirectNode * COS_Nullable
cos_indirect_node_create(CosAllocator *allocator,
                         CosObjID id,
                         CosNode * COS_Nullable value)
    COS_ALLOCATOR_FUNC
    COS_OWNERSHIP_HOLDS(3);

COS_API CosObjID
cos_indirect_node_get_id(const CosIndirectNode *node);

COS_API CosNode * COS_Nullable
cos_indirect_node_get_value(const CosIndirectNode *node);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_OBJECTS_NODES_COS_INDIRECT_NODE_H */
