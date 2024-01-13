/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/objects/CosBoolObj.h"

#include "common/Assert.h"
#include "libcos/CosBaseObj.h"
#include "libcos/private/objects/CosBoolObj-Impl.h"

COS_ASSUME_NONNULL_BEGIN

CosBoolObj *
cos_bool_obj_create(bool value)
{
    CosBoolObj * const obj = cos_obj_alloc(sizeof(CosBoolObj),
                                           CosObjectType_Boolean,
                                           NULL);
    if (!obj) {
        return NULL;
    }

    obj->value = value;

    return obj;
}

bool
cos_bool_obj_get_value(const CosBoolObj *bool_obj,
                       bool *value,
                       CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(bool_obj != NULL);
    COS_PARAMETER_ASSERT(value != NULL);

    if (!bool_obj) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Boolean object is NULL"),
                            error);
        return false;
    }
    else if (!value) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Value pointer is NULL"),
                            error);
        return false;
    }

    *value = bool_obj->value;

    return true;
}

bool
cos_bool_obj_set_value(CosBoolObj *bool_obj,
                       bool value,
                       CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(bool_obj != NULL);

    if (!bool_obj) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Boolean object is NULL"),
                            error);
        return false;
    }

    bool_obj->value = value;

    return true;
}

COS_ASSUME_NONNULL_END
