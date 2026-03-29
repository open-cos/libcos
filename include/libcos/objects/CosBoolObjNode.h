/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_OBJECTS_COS_BOOL_OBJ_NODE_H
#define LIBCOS_OBJECTS_COS_BOOL_OBJ_NODE_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

CosBoolObjNode * COS_Nullable
cos_bool_obj_node_alloc(bool value)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_bool_obj_node_free(CosBoolObjNode *bool_obj);

/**
 * @brief Get the value of a boolean object.
 *
 * @param bool_obj The boolean object.
 *
 * @return The value of the boolean object.
 */
bool
cos_bool_obj_node_get_value(const CosBoolObjNode *bool_obj);

/**
 * @brief Set the value of a boolean object.
 *
 * @param bool_obj The boolean object.
 * @param value The new value.
 */
void
cos_bool_obj_node_set_value(CosBoolObjNode *bool_obj,
                       bool value);

void
cos_bool_obj_node_print_desc(const CosBoolObjNode *bool_obj);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_OBJECTS_COS_BOOL_OBJ_NODE_H */
