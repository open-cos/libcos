/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_OBJECTS_COS_ARRAY_OBJ_H
#define LIBCOS_OBJECTS_COS_ARRAY_OBJ_H

#include <libcos/common/CosArray.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief The callbacks for arrays of objects.
 */
extern const CosArrayCallbacks cos_array_obj_callbacks;

CosArrayObj * COS_Nullable
cos_array_obj_alloc(CosArray * COS_Nullable array)
    COS_ATTR_MALLOC
    COS_ATTR_OWNERSHIP_RETURNS(malloc)
    COS_WARN_UNUSED_RESULT;

void
cos_array_obj_free(CosArrayObj *array_obj)
    COS_ATTR_OWNERSHIP_TAKES(malloc, 1);

/**
 * @brief Get the number of objects in the array.
 *
 * @param array_obj The array object.
 *
 * @return The number of objects in the array.
 */
size_t
cos_array_obj_get_count(const CosArrayObj *array_obj);

/**
 * @brief Get the object at the specified index.
 *
 * @param array_obj The array object.
 * @param index The index of the object to get.
 * @param out_error The error object to set if an error occurs.
 *
 * @return The object at the specified index, or @c NULL if an error occurred.
 */
CosObj * COS_Nullable
cos_array_obj_get_at(const CosArrayObj *array_obj,
                     size_t index,
                     CosError * COS_Nullable out_error);

bool
cos_array_obj_insert(CosArrayObj *array_obj,
                     size_t index,
                     CosObj *obj,
                     CosError * COS_Nullable error);

bool
cos_array_obj_append(CosArrayObj *array_obj,
                     CosObj *obj,
                     CosError * COS_Nullable error);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_OBJECTS_COS_ARRAY_OBJ_H */
