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

CosNameObj * COS_Nullable
cos_name_obj_alloc(CosString *value)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_name_obj_free(CosNameObj *name_obj);

CosString * COS_Nullable
cos_name_obj_get_value(const CosNameObj *name_obj);

void
cos_name_obj_set_value(CosNameObj *name_obj,
                       CosString *value);

void
cos_name_obj_print_desc(const CosNameObj *name_obj);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_NAME_OBJ_H */
