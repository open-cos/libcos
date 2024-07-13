/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_OBJECTS_NODES_COS_OBJ_NODE_H
#define LIBCOS_OBJECTS_NODES_COS_OBJ_NODE_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef enum CosObjNodeType {
    CosObjNodeType_Unknown = 0,

    CosObjNodeType_Document,
    CosObjNodeType_Array,
    CosObjNodeType_Dictionary,
    CosObjNodeType_Stream,
    CosObjNodeType_Indirect,
    CosObjNodeType_Reference,
    CosObjNodeType_Null,
    CosObjNodeType_Boolean,
    CosObjNodeType_Number,
    CosObjNodeType_String,
    CosObjNodeType_Name,
} CosObjNodeType;

/**
 * @brief Returns the type of the object node.
 *
 * @param node The object node.
 *
 * @return The type of the object node.
 */
CosObjNodeType
cos_obj_node_get_type(const CosObjNode *node);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_OBJECTS_NODES_COS_OBJ_NODE_H */
