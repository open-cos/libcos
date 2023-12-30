/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COS_NUMBER_OBJ_H
#define LIBCOS_COS_NUMBER_OBJ_H

#include <libcos/CosBaseObj.h>
#include <libcos/common/CosTypes.h>
#include <libcos/private/objects/CosBaseObj-Impl.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosIntegerObj {
    CosBaseObj base;

    int value;
};

CosIntegerObj * COS_Nullable
cos_integer_obj_alloc(int value)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_NUMBER_OBJ_H */
