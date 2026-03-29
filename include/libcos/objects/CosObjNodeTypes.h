/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_OBJECTS_COS_OBJ_NODE_TYPES_H
#define LIBCOS_OBJECTS_COS_OBJ_NODE_TYPES_H

#include <libcos/common/CosDefines.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef enum CosObjNodeValueType {
    CosObjNodeValueType_Unknown = 0,

    CosObjNodeValueType_Boolean,
    CosObjNodeValueType_Integer,
    CosObjNodeValueType_Real,
    CosObjNodeValueType_String,
    CosObjNodeValueType_Name,
    CosObjNodeValueType_Array,
    CosObjNodeValueType_Dict,
    CosObjNodeValueType_Stream,
    CosObjNodeValueType_Null,
} CosObjNodeValueType;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_OBJECTS_COS_OBJ_NODE_TYPES_H */
