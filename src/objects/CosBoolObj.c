/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/objects/CosBoolObj.h"

#include "common/Assert.h"
#include "libcos/objects/CosObj.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosBoolObj {
    CosObjType type;

    bool value;

    COS_ATTR_UNUSED unsigned char padding_[3] /* for alignment */;
};

CosBoolObj *
cos_bool_obj_alloc(bool value)
{
    CosBoolObj * const obj = calloc(1, sizeof(CosBoolObj));
    if (!obj) {
        return NULL;
    }

    obj->type = CosObjType_Boolean;
    obj->value = value;

    return obj;
}

void
cos_bool_obj_free(CosBoolObj *bool_obj)
{
    if (!bool_obj) {
        return;
    }

    free(bool_obj);
}

bool
cos_bool_obj_get_value(const CosBoolObj *bool_obj)
{
    COS_PARAMETER_ASSERT(bool_obj != NULL);
    if (!bool_obj) {
        return false;
    }

    return bool_obj->value;
}

void
cos_bool_obj_set_value(CosBoolObj *bool_obj,
                       bool value)
{
    COS_PARAMETER_ASSERT(bool_obj != NULL);
    if (!bool_obj) {
        return;
    }

    bool_obj->value = value;
}

COS_ASSUME_NONNULL_END
