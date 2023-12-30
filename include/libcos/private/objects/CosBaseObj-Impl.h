/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COS_OBJ_IMPL_H
#define LIBCOS_COS_OBJ_IMPL_H

#include "libcos/private/common/CosObj-Impl.h"

#include <libcos/CosBaseObj.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosBaseObj {
    CosObj base;

    CosDoc * COS_Nullable document;
};

CosClass *
cos_base_obj_class(void);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_OBJ_IMPL_H */
