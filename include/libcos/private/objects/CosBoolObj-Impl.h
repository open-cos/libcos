/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_PRIVATE_OBJECTS_COS_BOOL_OBJ_IMPL_H
#define LIBCOS_PRIVATE_OBJECTS_COS_BOOL_OBJ_IMPL_H

#include <libcos/CosObject.h>
#include <libcos/common/CosDefines.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosBoolObj {
    CosObjectType type;

    bool value;
};

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_PRIVATE_OBJECTS_COS_BOOL_OBJ_IMPL_H */
