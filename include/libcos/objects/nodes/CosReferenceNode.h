/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_NODES_COS_REFERENCE_NODE_H
#define LIBCOS_NODES_COS_REFERENCE_NODE_H

#include <libcos/CosObjID.h>
#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

COS_API CosReferenceNode * COS_Nullable
cos_reference_node_create(CosAllocator *allocator,
                          CosObjID id)
    COS_ALLOCATOR_FUNC;

/**
 * Returns the ID of the reference node.
 *
 * @param node The reference node.
 *
 * @return The ID of the reference node.
 */
COS_API CosObjID
cos_reference_node_get_id(const CosReferenceNode *node)
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_NODES_COS_REFERENCE_NODE_H */
