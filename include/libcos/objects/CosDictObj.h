/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COS_DICT_OBJ_H
#define LIBCOS_COS_DICT_OBJ_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

CosDictObj * COS_Nullable
cos_dict_obj_alloc(CosDict * COS_Nullable dict)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_dict_obj_free(CosDictObj *dict_obj);

bool
cos_dict_obj_set(CosDictObj *dict_obj,
                 CosNameObj *key,
                 CosObj *value,
                 CosError * COS_Nullable error);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_DICT_OBJ_H */
