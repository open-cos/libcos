/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_COS_BOOL_NODE_H
#define LIBCOS_COS_BOOL_NODE_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

COS_API CosBoolNode * COS_Nullable
cos_bool_node_create(CosAllocator *allocator,
                     bool value)
    COS_ALLOCATOR_FUNC;

COS_API bool
cos_bool_node_get_value(const CosBoolNode *bool_node);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_BOOL_NODE_H */
