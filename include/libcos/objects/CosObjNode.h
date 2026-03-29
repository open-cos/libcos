/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_OBJECTS_COS_OBJ_NODE_H
#define LIBCOS_OBJECTS_COS_OBJ_NODE_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>
#include <libcos/objects/CosObjNodeTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef enum CosObjNodeType {
    CosObjNodeType_Unknown = 0,

    CosObjNodeType_Boolean,
    CosObjNodeType_Integer,
    CosObjNodeType_Real,
    CosObjNodeType_String,
    CosObjNodeType_Name,
    CosObjNodeType_Array,
    CosObjNodeType_Dict,
    CosObjNodeType_Stream,
    CosObjNodeType_Null,

    CosObjNodeType_Indirect,
    CosObjNodeType_Reference,
} CosObjNodeType;

/**
 * @brief Retains an object, incrementing its reference count.
 *
 * @param obj The object to retain.
 *
 * @return The retained object (same pointer as @p obj).
 */
CosObjNode * COS_Nullable
cos_obj_node_retain(CosObjNode * COS_Nullable obj);

/**
 * @brief Releases an object, decrementing its reference count.
 *
 * When the reference count reaches zero the object is deallocated.
 *
 * @param obj The object to release.
 */
void
cos_obj_node_free(CosObjNode * COS_Nullable obj);

/** @name Object type **/
/** @{ **/

CosObjNodeType
cos_obj_node_get_type(const CosObjNode *obj);

/**
 * @brief Determines whether an object is direct.
 *
 * @param obj The object.
 *
 * @return @c true if the object is direct, @c false otherwise.
 */
bool
cos_obj_node_is_direct(const CosObjNode *obj);

/**
 * @brief Determines whether an object is indirect.
 *
 * @param obj The object.
 *
 * @return @c true if the object is indirect, @c false otherwise.
 */
bool
cos_obj_node_is_indirect(const CosObjNode *obj);

/**
 * @brief Determines whether an object is of a specific type.
 *
 * @param obj The object.
 * @param type The type to check for.
 *
 * @return @c true if the object is of the specified type, @c false otherwise.
 */
bool
cos_obj_node_is_type(const CosObjNode *obj,
                CosObjNodeType type);

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
CosObjNodeValueType
cos_obj_node_get_value_type(CosObjNode *obj);

/**
 * @brief Determines whether an object is a boolean.
 *
 * @param obj The object.
 *
 * @return @c true if the object is a boolean, @c false otherwise.
 */
bool
cos_obj_node_is_boolean(CosObjNode *obj);

/**
 * @brief Determines whether an object is an integer.
 *
 * @param obj The object.
 *
 * @return @c true if the object is an integer, @c false otherwise.
 */
bool
cos_obj_node_is_integer(CosObjNode *obj);

/**
 * @brief Determines whether an object is a real number.
 *
 * @param obj The object.
 *
 * @return @c true if the object is a real number, @c false otherwise.
 */
bool
cos_obj_node_is_real(CosObjNode *obj);

/**
 * @brief Determines whether an object is a string.
 *
 * @param obj The object.
 *
 * @return @c true if the object is a string, @c false otherwise.
 */
bool
cos_obj_node_is_string(CosObjNode *obj);

/**
 * @brief Determines whether an object is a name.
 *
 * @param obj The object.
 *
 * @return @c true if the object is a name, @c false otherwise.
 */
bool
cos_obj_node_is_name(CosObjNode *obj);

/**
 * @brief Determines whether an object is an array.
 *
 * @param obj The object.
 *
 * @return @c true if the object is an array, @c false otherwise.
 */
bool
cos_obj_node_is_array(CosObjNode *obj);

/**
 * @brief Determines whether an object is a dictionary.
 *
 * @param obj The object.
 *
 * @return @c true if the object is a dictionary, @c false otherwise.
 */
bool
cos_obj_node_is_dict(CosObjNode *obj);

/**
 * @brief Determines whether an object is a stream.
 *
 * @param obj The object.
 *
 * @return @c true if the object is a stream, @c false otherwise.
 */
bool
cos_obj_node_is_stream(CosObjNode *obj);

/**
 * @brief Determines whether an object is null.
 *
 * @param obj The object.
 *
 * @return @c true if the object is null, @c false otherwise.
 */
bool
cos_obj_node_is_null(CosObjNode *obj);

/** @} **/

/** @name Debugging **/
/** @{ **/

void
cos_obj_node_print_desc(const CosObjNode *obj);

/** @} **/

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_OBJECTS_COS_OBJ_NODE_H */
