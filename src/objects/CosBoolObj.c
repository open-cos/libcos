//
// Created by david on 15/10/23.
//

#include "libcos/CosBoolObj.h"

#include "CosBoolObj-Impl.h"
#include "common/Assert.h"
#include "libcos/CosObj.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

CosBoolObj *
cos_bool_obj_alloc(void)
{
    CosBoolObj * const obj = cos_obj_alloc(sizeof(CosBoolObj),
                                           CosObjectType_Boolean,
                                           NULL);
    if (!obj) {
        return NULL;
    }

    obj->value = false;

    return obj;
}

bool
cos_bool_obj_get_value(const CosBoolObj *bool_obj,
                       bool *value,
                       CosError ** COS_Nullable error)
{
    COS_PARAMETER_ASSERT(bool_obj != NULL);
    COS_PARAMETER_ASSERT(value != NULL);

    if (!bool_obj) {
        cos_error_propagate(cos_error_alloc(COS_ERROR_INVALID_ARGUMENT,
                                            "Boolean object is NULL"),
                            error);
        return false;
    }
    else if (!value) {
        cos_error_propagate(cos_error_alloc(COS_ERROR_INVALID_ARGUMENT,
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
                       CosError ** COS_Nullable error)
{
    COS_PARAMETER_ASSERT(bool_obj != NULL);

    if (!bool_obj) {
        cos_error_propagate(cos_error_alloc(COS_ERROR_INVALID_ARGUMENT,
                                            "Boolean object is NULL"),
                            error);
        return false;
    }

    bool_obj->value = value;

    return true;
}

COS_ASSUME_NONNULL_END
