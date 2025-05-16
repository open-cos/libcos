/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_NODES_COS_STREAM_NODE_H
#define LIBCOS_NODES_COS_STREAM_NODE_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

COS_API CosStreamNode * COS_Nullable
cos_stream_node_create(CosAllocator *allocator,
                       CosDictNode *dict,
                       CosData *data)
    COS_ALLOCATOR_FUNC
    COS_OWNERSHIP_HOLDS(2)
    COS_OWNERSHIP_HOLDS(3);

COS_API CosDictNode * COS_Nullable
cos_stream_node_get_dict(const CosStreamNode *node)
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_NODES_COS_STREAM_NODE_H */
