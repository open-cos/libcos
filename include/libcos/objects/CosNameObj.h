/*
 * Copyright (c) 2024 OpenCOS.
 */

//
// Created by david on 15/10/23.
//

#ifndef LIBCOS_COS_NAME_OBJ_H
#define LIBCOS_COS_NAME_OBJ_H

#include "libcos/common/CosDefines.h"
#include "libcos/common/CosError.h"
#include "libcos/common/CosTypes.h"

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

void
cos_name_obj_free(CosNameObj *name_obj)
    COS_DEALLOCATOR_FUNC;

CosNameObj * COS_Nullable
cos_name_obj_alloc(CosString *value)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_name_obj_free)
    COS_OWNERSHIP_HOLDS(1);

const CosString * COS_Nullable
cos_name_obj_get_value(const CosNameObj *name_obj);

void
cos_name_obj_set_value(CosNameObj *name_obj,
                       CosString *value);

/**
 * @brief Returns the hash value of the name object.
 *
 * @param name_obj The name object.
 *
 * @return The hash value of the name object.
 */
size_t
cos_name_obj_get_hash(const CosNameObj *name_obj);

/**
 * @brief Returns whether two name objects are equal.
 *
 * @param name_obj1 The first name object.
 * @param name_obj2 The second name object.
 *
 * @return @c true if the name objects are equal, otherwise @c false .
 */
bool
cos_name_obj_equal(const CosNameObj *name_obj1,
                   const CosNameObj *name_obj2);

void
cos_name_obj_print_desc(const CosNameObj *name_obj);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_NAME_OBJ_H */
