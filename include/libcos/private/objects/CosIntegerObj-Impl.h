/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_PRIVATE_OBJECTS_COS_INTEGER_OBJ_IMPL_H
#define LIBCOS_PRIVATE_OBJECTS_COS_INTEGER_OBJ_IMPL_H

#include <libcos/CosObject.h>
#include <libcos/common/CosDefines.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosIntegerObj {
    CosObjectType type;

    int value;
};

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_PRIVATE_OBJECTS_COS_INTEGER_OBJ_IMPL_H */
