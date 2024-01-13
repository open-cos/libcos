/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_PRIVATE_OBJECTS_COS_NULL_OBJ_IMPL_H
#define LIBCOS_PRIVATE_OBJECTS_COS_NULL_OBJ_IMPL_H

#include <libcos/common/CosDefines.h>
#include <libcos/private/objects/CosDirectObj-Impl.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosNullObj {
    CosDirectObj base;
};

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_PRIVATE_OBJECTS_COS_NULL_OBJ_IMPL_H */
