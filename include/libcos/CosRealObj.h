/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COS_REAL_OBJ_H
#define LIBCOS_COS_REAL_OBJ_H

#include <libcos/CosBaseObj.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>
#include <libcos/private/objects/CosBaseObj-Impl.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosRealObj {
    CosBaseObj base;

    double value;
};

CosRealObj * COS_Nullable
cos_real_obj_alloc(double value,
                   CosDocument * COS_Nullable document)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_REAL_OBJ_H */
