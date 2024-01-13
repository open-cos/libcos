/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COS_OBJECT_H
#define LIBCOS_COS_OBJECT_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef enum CosObjectType {
    CosObjectType_Unknown = 0,

    CosObjectType_Boolean,
    CosObjectType_Integer,
    CosObjectType_Real,
    CosObjectType_String,
    CosObjectType_Name,
    CosObjectType_Array,
    CosObjectType_Dictionary,
    CosObjectType_Stream,
    CosObjectType_Null,
} CosObjectType;

/**
 * @brief Returns the type of the object.
 *
 * @param obj The object.
 *
 * @return The type of the object.
 */
CosObjectType
cos_object_get_type(CosObject *obj);

/**
 * @brief Returns whether the object is a direct object.
 *
 * @param obj The object.
 *
 * @return @c true if the object is a direct object, @c false otherwise.
 */
bool
cos_object_is_direct(const CosObject *obj);

/**
 * @brief Returns whether the object is an indirect object.
 *
 * @param obj The object.
 *
 * @return @c true if the object is an indirect object, @c false otherwise.
 */
bool
cos_object_is_indirect(const CosObject *obj);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_OBJECT_H */
