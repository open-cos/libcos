/*
 * Copyright (c) 2023-2024 OpenCOS.
 */

#ifndef LIBCOS_COS_NUMBER_OBJ_H
#define LIBCOS_COS_NUMBER_OBJ_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

CosIntObj * COS_Nullable
cos_int_obj_alloc(int value)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_int_obj_free(CosIntObj *int_obj);

#pragma mark - Value accessors

int
cos_int_obj_get_value(const CosIntObj *int_obj);

void
cos_int_obj_set_value(CosIntObj *int_obj,
                          int value);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_NUMBER_OBJ_H */
