/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_OBJECTS_NODES_COS_NODE_H
#define LIBCOS_OBJECTS_NODES_COS_NODE_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef enum CosNodeType {
    CosNodeType_Unknown = 0,

    CosNodeType_Array,
    CosNodeType_Dictionary,
    CosNodeType_Stream,
    CosNodeType_Indirect,
    CosNodeType_Reference,
    CosNodeType_Null,
    CosNodeType_Boolean,
    CosNodeType_Integer,
    CosNodeType_Real,
    CosNodeType_String,
    CosNodeType_Name,
} CosNodeType;

/**
 * @brief Returns the type of the object node.
 *
 * @param node The object node.
 *
 * @return The type of the object node.
 */
COS_API CosNodeType
cos_obj_node_get_type(const CosNode *node);

COS_API CosNode * COS_Nullable
cos_node_get_parent(const CosNode *node);

// MARK: - Type checkers

COS_API bool
cos_node_is_bool(const CosNode *node);

COS_API bool
cos_node_is_number(const CosNode *node);

COS_API bool
cos_node_is_integer(const CosNode *node);

COS_API bool
cos_node_is_real(const CosNode *node);

COS_API bool
cos_node_is_string(const CosNode *node);

COS_API bool
cos_node_is_name(const CosNode *node);

COS_API bool
cos_node_is_array(const CosNode *node);

COS_API bool
cos_node_is_dict(const CosNode *node);

COS_API bool
cos_node_is_stream(const CosNode *node);

COS_API bool
cos_node_is_indirect(const CosNode *node);

COS_API bool
cos_node_is_reference(const CosNode *node);

COS_API bool
cos_node_is_null(const CosNode *node);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_OBJECTS_NODES_COS_NODE_H */
