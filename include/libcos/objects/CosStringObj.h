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

void
cos_string_obj_free(CosStringObj *string_obj);

CosData * COS_Nullable
cos_string_obj_get_value(const CosStringObj *string_obj);

void
cos_string_obj_set_value(CosStringObj *string_obj,
                         CosData *data);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_STRING_OBJ_H */
