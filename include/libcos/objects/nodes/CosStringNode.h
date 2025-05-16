/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_NODES_COS_STRING_NODE_H
#define LIBCOS_NODES_COS_STRING_NODE_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

COS_API CosStringNode * COS_Nullable
cos_string_node_create(CosAllocator *allocator,
                       CosData *value)
    COS_ALLOCATOR_FUNC
    COS_OWNERSHIP_HOLDS(2);

COS_API CosData * COS_Nullable
cos_string_node_get_value(const CosStringNode *string_node)
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_NODES_COS_STRING_NODE_H */
