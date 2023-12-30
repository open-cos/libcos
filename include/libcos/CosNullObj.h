/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COS_NULL_OBJ_H
#define LIBCOS_COS_NULL_OBJ_H

#include <libcos/CosBaseObj.h>
#include <libcos/private/objects/CosBaseObj-Impl.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosNullObj {
    CosBaseObj base;
};

CosNullObj *
cos_null_obj_get(void);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_NULL_OBJ_H */
