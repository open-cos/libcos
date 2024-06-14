/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COS_DICT_OBJ_H
#define LIBCOS_COS_DICT_OBJ_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosString.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief Destroys a dictionary object.
 *
 * @param dict_obj The dictionary object.
 */
void
cos_dict_obj_destroy(CosDictObj *dict_obj)
    COS_DEALLOCATOR_FUNC;

/**
 * @brief Creates a dictionary object.
 *
 * @param dict The dictionary or @c NULL.
 *
 * @return The dictionary object, or @c NULL if an error occurred.
 */
CosDictObj * COS_Nullable
cos_dict_obj_create(CosDict * COS_Nullable dict)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_dict_obj_destroy);

/**
 * @brief Gets the number of entries in a dictionary object.
 *
 * @param dict_obj The dictionary object.
 *
 * @return The number of entries in the dictionary object.
 */
size_t
cos_dict_obj_get_count(const CosDictObj *dict_obj);

/**
 * @brief Gets the object value for a given key in a dictionary object.
 *
 * @param dict_obj The dictionary object.
 * @param key The key.
 * @param out_value The output parameter for the object value.
 * @param out_error The error information.
 *
 * @return @c true if the object value was found, otherwise @c false.
 */
bool
cos_dict_obj_get_value(const CosDictObj *dict_obj,
                       CosStringRef key,
                       CosObj * COS_Nullable * COS_Nonnull out_value,
                       CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(3)
    COS_ATTR_ACCESS_WRITE_ONLY(4);

bool
cos_dict_obj_set(CosDictObj *dict_obj,
                 CosNameObj *key,
                 CosObj *value,
                 CosError * COS_Nullable error);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_DICT_OBJ_H */
