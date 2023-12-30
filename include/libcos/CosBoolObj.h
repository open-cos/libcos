//
// Created by david on 15/10/23.
//

#ifndef LIBCOS_COS_BOOL_OBJ_H
#define LIBCOS_COS_BOOL_OBJ_H

#include <libcos/CosBaseObj.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosTypes.h>
#include <libcos/private/objects/CosBaseObj-Impl.h>

#include <stdbool.h>

COS_ASSUME_NONNULL_BEGIN

struct CosBoolObj {
    CosBaseObj base;

    bool value;
};

CosBoolObj * COS_Nullable
cos_bool_obj_alloc(bool value)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

bool
cos_bool_obj_get_value(const CosBoolObj *bool_obj,
                       bool *value,
                       CosError * COS_Nullable error);

bool
cos_bool_obj_set_value(CosBoolObj *bool_obj,
                       bool value,
                       CosError * COS_Nullable error);

COS_ASSUME_NONNULL_END

#endif /* LIBCOS_COS_BOOL_OBJ_H */
