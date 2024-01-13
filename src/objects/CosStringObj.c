/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/objects/CosStringObj.h"

#include "common/Assert.h"
#include "libcos/private/objects/CosStringObj-Impl.h"

#include <libcos/CosBaseObj.h>
#include <libcos/common/CosData.h>

COS_ASSUME_NONNULL_BEGIN

CosStringObj * COS_Nullable
cos_string_obj_alloc(CosData *data)
{
    CosStringObj * const obj = cos_obj_alloc(sizeof(CosStringObj),
                                             CosObjectType_String,
                                             NULL);
    if (!obj) {
        return NULL;
    }

    obj->data = data;

    return obj;
}

bool
cos_string_obj_set_data(CosStringObj *string_obj,
                        CosData *data,
                        CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(string_obj != NULL);
    COS_PARAMETER_ASSERT(data != NULL);
    if (!string_obj || !data) {
        return false;
    }

    if (string_obj->data) {
        cos_data_free(string_obj->data);
    }

    string_obj->data = data;

    return true;
}

bool
cos_string_get_data(const CosStringObj *string_obj,
                    const CosData * COS_Nullable *data,
                    CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(string_obj != NULL);
    COS_PARAMETER_ASSERT(data != NULL);
    if (!string_obj || !data) {
        return false;
    }

    *data = string_obj->data;

    return true;
}

COS_ASSUME_NONNULL_END
