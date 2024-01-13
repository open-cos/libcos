/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COS_STRING_OBJ_H
#define LIBCOS_COS_STRING_OBJ_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

CosStringObj * COS_Nullable
cos_string_obj_alloc(CosData *data)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

bool
cos_string_obj_set_data(CosStringObj *string_obj,
                        CosData *data,
                        CosError * COS_Nullable error);

bool
cos_string_get_data(const CosStringObj *string_obj,
                    const CosData * COS_Nullable *data,
                    CosError * COS_Nullable error);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_STRING_OBJ_H */
