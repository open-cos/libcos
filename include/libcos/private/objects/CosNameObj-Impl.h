/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COS_NAME_OBJ_IMPL_H
#define LIBCOS_COS_NAME_OBJ_IMPL_H

#include "libcos/private/common/CosClass-Impl.h"

#include <libcos/CosNameObj.h>
#include <libcos/common/CosDefines.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosNameObj {
    CosBaseObj base;

    CosString *value;
};

typedef struct CosNameObjClass {
    CosClass base;

    void (*init)(CosNameObj *name_obj,
                 CosString *value);
} CosNameObjClass;

CosClass *
cos_name_obj_class(void);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_NAME_OBJ_IMPL_H */
