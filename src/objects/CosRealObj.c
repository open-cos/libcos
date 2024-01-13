/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/objects/CosRealObj.h"

#include "libcos/CosBaseObj.h"
#include "libcos/private/objects/CosRealObj-Impl.h"

COS_ASSUME_NONNULL_BEGIN

CosRealObj *
cos_real_obj_alloc(double value,
                   CosDoc * COS_Nullable document)
{
    CosRealObj *result = cos_obj_alloc(sizeof(CosRealObj),
                                       CosObjectType_Real,
                                       document);
    if (!result) {
        return NULL;
    }

    result->value = value;

    return result;
}

COS_ASSUME_NONNULL_END
