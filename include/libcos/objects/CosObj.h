/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_OBJECTS_COS_OBJ_H
#define LIBCOS_OBJECTS_COS_OBJ_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>
#include <libcos/objects/CosObjectTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef enum CosObjType {
    CosObjType_Unknown = 0,

    CosObjType_Boolean,
    CosObjType_Integer,
    CosObjType_Real,
    CosObjType_String,
    CosObjType_Name,
    CosObjType_Array,
    CosObjType_Dict,
    CosObjType_Stream,
    CosObjType_Null,

    CosObjType_Indirect,
    CosObjType_Reference,
} CosObjType;

void
cos_obj_free(CosObj *obj)
    COS_DEALLOCATOR_FUNC;

/** @name Object type **/
/** @{ **/

CosObjType
cos_obj_get_type(const CosObj *obj);

/**
 * @brief Determines whether an object is direct.
 *
 * @param obj The object.
 *
 * @return @c true if the object is direct, @c false otherwise.
 */
bool
cos_obj_is_direct(const CosObj *obj);

/**
 * @brief Determines whether an object is indirect.
 *
 * @param obj The object.
 *
 * @return @c true if the object is indirect, @c false otherwise.
 */
bool
cos_obj_is_indirect(const CosObj *obj);

/**
 * @brief Determines whether an object is of a specific type.
 *
 * @param obj The object.
 * @param type The type to check for.
 *
 * @return @c true if the object is of the specified type, @c false otherwise.
 */
bool
cos_obj_is_type(const CosObj *obj,
                CosObjType type);

/**
 * @brief Gets the value type of an object.
 *
 * For indirect objects, this function will return the value type of the
 * resolved object.
 *
 * @param obj The object.
 *
 * @return The value type of the object.
 */
CosObjValueType
cos_obj_get_value_type(CosObj *obj);

/**
 * @brief Determines whether an object is a boolean.
 *
 * @param obj The object.
 *
 * @return @c true if the object is a boolean, @c false otherwise.
 */
bool
cos_obj_is_boolean(CosObj *obj);

/**
 * @brief Determines whether an object is an integer.
 *
 * @param obj The object.
 *
 * @return @c true if the object is an integer, @c false otherwise.
 */
bool
cos_obj_is_integer(CosObj *obj);

/**
 * @brief Determines whether an object is a real number.
 *
 * @param obj The object.
 *
 * @return @c true if the object is a real number, @c false otherwise.
 */
bool
cos_obj_is_real(CosObj *obj);

/**
 * @brief Determines whether an object is a string.
 *
 * @param obj The object.
 *
 * @return @c true if the object is a string, @c false otherwise.
 */
bool
cos_obj_is_string(CosObj *obj);

/**
 * @brief Determines whether an object is a name.
 *
 * @param obj The object.
 *
 * @return @c true if the object is a name, @c false otherwise.
 */
bool
cos_obj_is_name(CosObj *obj);

/**
 * @brief Determines whether an object is an array.
 *
 * @param obj The object.
 *
 * @return @c true if the object is an array, @c false otherwise.
 */
bool
cos_obj_is_array(CosObj *obj);

/**
 * @brief Determines whether an object is a dictionary.
 *
 * @param obj The object.
 *
 * @return @c true if the object is a dictionary, @c false otherwise.
 */
bool
cos_obj_is_dict(CosObj *obj);

/**
 * @brief Determines whether an object is a stream.
 *
 * @param obj The object.
 *
 * @return @c true if the object is a stream, @c false otherwise.
 */
bool
cos_obj_is_stream(CosObj *obj);

/**
 * @brief Determines whether an object is null.
 *
 * @param obj The object.
 *
 * @return @c true if the object is null, @c false otherwise.
 */
bool
cos_obj_is_null(CosObj *obj);

/** @} **/

/** @name Debugging **/
/** @{ **/

void
cos_obj_print_desc(const CosObj *obj);

/** @} **/

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_OBJECTS_COS_OBJ_H */
