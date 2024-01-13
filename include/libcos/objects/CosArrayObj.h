/*
 * Copyright (c) 2024 OpenCOS.
 */

//
// Created by david on 15/10/23.
//

#ifndef LIBCOS_COS_ARRAY_OBJ_H
#define LIBCOS_COS_ARRAY_OBJ_H

#include "libcos/CosBaseObj.h"
#include "libcos/common/CosDefines.h"
#include "libcos/common/CosError.h"
#include "libcos/common/CosTypes.h"

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

CosArrayObj * COS_Nullable
cos_array_obj_create(void)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

bool
cos_array_object_append(CosArrayObj *array_obj,
                        CosBaseObj *object,
                        CosError * COS_Nullable error);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_ARRAY_OBJ_H */
