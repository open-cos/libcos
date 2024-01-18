/*
 * Copyright (c) 2024 OpenCOS.
 */

//
// Created by david on 15/10/23.
//

#ifndef LIBCOS_COS_ARRAY_OBJ_H
#define LIBCOS_COS_ARRAY_OBJ_H

#include <libcos/common/CosArray.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

extern const CosArrayCallbacks cos_array_obj_callbacks;

CosArrayObj * COS_Nullable
cos_array_obj_alloc(CosArray * COS_Nullable array)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_array_obj_free(CosArrayObj *array_obj);

const CosArray * COS_Nullable
cos_array_obj_get_array(const CosArrayObj *array_obj);

bool
cos_array_object_append(CosArrayObj *array_obj,
                        CosObj *obj,
                        CosError * COS_Nullable error);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_ARRAY_OBJ_H */
