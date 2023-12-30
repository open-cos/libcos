/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COS_DICT_OBJ_IMPL_H
#define LIBCOS_COS_DICT_OBJ_IMPL_H

#include "libcos/private/common/CosClass-Impl.h"

#include <libcos/common/CosDefines.h>
#include <libcos/private/objects/CosBaseObj-Impl.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef struct CosDict CosDict;

struct CosDictObj {
    CosBaseObj base;

    CosDict *dict;
};

typedef struct CosDictObjClass {
    CosClass base;

    void (*init)(CosDictObj *dict_obj,
                 CosDict *dict);
} CosDictObjClass;

CosClass *
cos_dict_obj_class(void);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_DICT_OBJ_IMPL_H */
