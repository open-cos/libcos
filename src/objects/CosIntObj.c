/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/objects/CosIntObj.h"

#include "common/Assert.h"
#include "libcos/objects/CosObj.h"

#include <stddef.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosIntObj {
    CosObjType type;

    int value;
};

CosIntObj *
cos_int_obj_alloc(int value)
{
    CosIntObj * const int_obj = calloc(1, sizeof(CosIntObj));
    if (!int_obj) {
        return NULL;
    }

    int_obj->type = CosObjType_Integer;
    int_obj->value = value;

    return int_obj;
}

void
cos_int_obj_free(CosIntObj *int_obj)
{
    if (!int_obj) {
        return;
    }

    free(int_obj);
}

int
cos_int_obj_get_value(const CosIntObj *int_obj)
{
    COS_PARAMETER_ASSERT(int_obj != NULL);
    if (!int_obj) {
        return 0;
    }

    return int_obj->value;
}

void
cos_int_obj_set_value(CosIntObj *int_obj,
                      int value)
{
    COS_PARAMETER_ASSERT(int_obj != NULL);
    if (!int_obj) {
        return;
    }

    int_obj->value = value;
}

COS_ASSUME_NONNULL_END
