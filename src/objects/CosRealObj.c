/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/objects/CosRealObj.h"

#include "common/Assert.h"
#include "libcos/objects/CosObj.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosRealObj {
    CosObjType type;

    double value;
};

CosRealObj *
cos_real_obj_alloc(double value)
{
    CosRealObj *result = calloc(1, sizeof(CosRealObj));
    if (!result) {
        return NULL;
    }

    result->type = CosObjType_Real;
    result->value = value;

    return result;
}

void
cos_real_obj_free(CosRealObj *real_obj)
{
    if (!real_obj) {
        return;
    }

    free(real_obj);
}

double
cos_real_obj_get_value(const CosRealObj *real_obj)
{
    COS_PARAMETER_ASSERT(real_obj != NULL);
    if (!real_obj) {
        return 0.0;
    }

    return real_obj->value;
}

void
cos_real_obj_set_value(CosRealObj *real_obj,
                       double value)
{
    COS_PARAMETER_ASSERT(real_obj != NULL);
    if (!real_obj) {
        return;
    }

    real_obj->value = value;
}

COS_ASSUME_NONNULL_END
