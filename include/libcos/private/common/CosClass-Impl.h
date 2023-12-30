/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_PRIVATE_COMMON_COS_CLASS_IMPL_H
#define LIBCOS_PRIVATE_COMMON_COS_CLASS_IMPL_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosClass {
    CosClass * COS_Nullable super;

    void (* COS_Nullable init)(CosObj *self);
    void (* COS_Nullable dealloc)(CosObj *self);
};

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_PRIVATE_COMMON_COS_CLASS_IMPL_H */
