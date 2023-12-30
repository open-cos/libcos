/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_PRIVATE_COMMON_COS_OBJ_IMPL_H
#define LIBCOS_PRIVATE_COMMON_COS_OBJ_IMPL_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosObj.h>

#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosObj {
    CosClass *class;

    volatile size_t ref_count;
};

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_PRIVATE_COMMON_COS_OBJ_IMPL_H */
