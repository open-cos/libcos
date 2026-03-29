/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/objects/CosRealObjNode.h"

#include "common/Assert.h"

#include "libcos/objects/CosObjNode.h"

#include <stdio.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosRealObjNode {
    CosObjNodeType type;
    unsigned int ref_count;

    double value;
};

CosRealObjNode *
cos_real_obj_node_alloc(double value)
{
    CosRealObjNode *result = calloc(1, sizeof(CosRealObjNode));
    if (!result) {
        return NULL;
    }

    result->type = CosObjNodeType_Real;
    result->ref_count = 1;
    result->value = value;

    return result;
}

void
cos_real_obj_node_free(CosRealObjNode *real_obj)
{
    if (!real_obj) {
        return;
    }

    free(real_obj);
}

double
cos_real_obj_node_get_value(const CosRealObjNode *real_obj)
{
    COS_API_PARAM_CHECK(real_obj != NULL);
    if (!real_obj) {
        return 0.0;
    }

    return real_obj->value;
}

void
cos_real_obj_node_set_value(CosRealObjNode *real_obj,
                       double value)
{
    COS_API_PARAM_CHECK(real_obj != NULL);
    if (!real_obj) {
        return;
    }

    real_obj->value = value;
}

void
cos_real_obj_node_print_desc(const CosRealObjNode *real_obj)
{
    COS_API_PARAM_CHECK(real_obj != NULL);
    if (!real_obj) {
        return;
    }

    printf("Real: %f\n", real_obj->value);
}

COS_ASSUME_NONNULL_END
