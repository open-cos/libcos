/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/CosRealObj.h"

COS_ASSUME_NONNULL_BEGIN

CosRealObj * COS_Nullable
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
