/*
 * Copyright (c) 2023-2024 OpenCOS.
 */

#ifndef LIBCOS_COS_REAL_OBJ_H
#define LIBCOS_COS_REAL_OBJ_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

CosRealObj * COS_Nullable
cos_real_obj_alloc(double value)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_real_obj_free(CosRealObj *real_obj);

#pragma mark - Value accessors

double
cos_real_obj_get_value(const CosRealObj *real_obj);

void
cos_real_obj_set_value(CosRealObj *real_obj,
                       double value);

void
cos_real_obj_print_desc(const CosRealObj *real_obj);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_REAL_OBJ_H */
