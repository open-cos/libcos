/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/objects/CosIntegerObj.h"

#include "common/Assert.h"
#include "libcos/CosBaseObj.h"
#include "libcos/private/objects/CosIntegerObj-Impl.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

static void
cos_integer_obj_init(CosIntegerObj *int_obj,
                     int value);

CosIntegerObj * COS_Nullable
cos_integer_obj_alloc(int value)
{
    CosIntegerObj *int_obj = (CosIntegerObj *)cos_obj_alloc(sizeof(CosIntegerObj),
                                                            CosObjectType_Integer,
                                                            NULL);
    if (!int_obj) {
        return NULL;
    }

    cos_integer_obj_init(int_obj, value);

    return int_obj;
}

static void
cos_integer_obj_init(CosIntegerObj *int_obj,
                     int value)
{
    COS_PARAMETER_ASSERT(int_obj != NULL);
    if (!int_obj) {
        return;
    }

    int_obj->value = value;
}

COS_ASSUME_NONNULL_END
