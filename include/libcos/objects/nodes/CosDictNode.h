/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_NODES_COS_DICT_NODE_H
#define LIBCOS_NODES_COS_DICT_NODE_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

COS_API CosDictNode * COS_Nullable
cos_dict_node_create(CosAllocator *allocator,
                     CosDict *value)
    COS_ALLOCATOR_FUNC
    COS_OWNERSHIP_HOLDS(2);

/**
 * Returns the value of the dictionary node.
 *
 * @param dict_node The dictionary node.
 *
 * @return The value of the dictionary node.
 */
COS_API CosDict * COS_Nullable
cos_dict_node_get_value(const CosDictNode *dict_node)
    COS_WARN_UNUSED_RESULT;

COS_API bool
cos_dict_node_get_child_count(CosDictNode *dict_node,
                              size_t *out_count);

COS_API CosNode * COS_Nullable
cos_dict_node_get_child(CosDictNode *dict_node,
                        CosString *key,
                        CosError * COS_Nullable out_error)
    COS_WARN_UNUSED_RESULT
    COS_ATTR_ACCESS_WRITE_ONLY(3);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_NODES_COS_DICT_NODE_H */
