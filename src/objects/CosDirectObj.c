/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/objects/CosDirectObj.h"

#include "common/Assert.h"
#include "libcos/private/objects/CosDirectObj-Impl.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

CosObjectType
cos_direct_obj_get_type(const CosDirectObj *direct_obj)
{
    COS_PARAMETER_ASSERT(direct_obj != NULL);
    if (!direct_obj) {
        return CosObjectType_Unknown;
    }

    return direct_obj->type;
}

COS_ASSUME_NONNULL_END
