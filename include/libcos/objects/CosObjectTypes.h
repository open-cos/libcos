/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_OBJECTS_COS_OBJECT_TYPES_H
#define LIBCOS_OBJECTS_COS_OBJECT_TYPES_H

#include <libcos/common/CosDefines.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef enum CosObjValueType {
    CosObjValueType_Unknown = 0,

    CosObjValueType_Boolean,
    CosObjValueType_Integer,
    CosObjValueType_Real,
    CosObjValueType_String,
    CosObjValueType_Name,
    CosObjValueType_Array,
    CosObjValueType_Dict,
    CosObjValueType_Stream,
    CosObjValueType_Null,
} CosObjValueType;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_OBJECTS_COS_OBJECT_TYPES_H */
