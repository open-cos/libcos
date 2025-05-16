/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_NODES_COS_INTEGER_NODE_H
#define LIBCOS_NODES_COS_INTEGER_NODE_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

COS_API CosIntegerNode * COS_Nullable
cos_integer_node_create(CosAllocator *allocator,
                        int value)
    COS_ALLOCATOR_FUNC;

/**
 * Returns the value of the integer node.
 *
 * @param node The integer node.
 *
 * @return The value of the integer node.
 */
COS_API int
cos_integer_node_get_value(const CosIntegerNode *node)
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_NODES_COS_INTEGER_NODE_H */
