/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/objects/CosBoolObjNode.h"

#include "common/Assert.h"

#include "libcos/objects/CosObjNode.h"

#include <stdio.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosBoolObjNode {
    CosObjNodeType type;
    unsigned int ref_count;

    bool value;
};

CosBoolObjNode *
cos_bool_obj_node_alloc(bool value)
{
    CosBoolObjNode * const obj = calloc(1, sizeof(CosBoolObjNode));
    if (!obj) {
        return NULL;
    }

    obj->type = CosObjNodeType_Boolean;
    obj->ref_count = 1;
    obj->value = value;

    return obj;
}

void
cos_bool_obj_node_free(CosBoolObjNode *bool_obj)
{
    if (!bool_obj) {
        return;
    }

    free(bool_obj);
}

bool
cos_bool_obj_node_get_value(const CosBoolObjNode *bool_obj)
{
    COS_API_PARAM_CHECK(bool_obj != NULL);
    if (!bool_obj) {
        return false;
    }

    return bool_obj->value;
}

void
cos_bool_obj_node_set_value(CosBoolObjNode *bool_obj,
                       bool value)
{
    COS_API_PARAM_CHECK(bool_obj != NULL);
    if (!bool_obj) {
        return;
    }

    bool_obj->value = value;
}

void
cos_bool_obj_node_print_desc(const CosBoolObjNode *bool_obj)
{
    COS_API_PARAM_CHECK(bool_obj != NULL);
    if (!bool_obj) {
        return;
    }

    printf("Boolean: %s\n", (bool_obj->value ? "true" : "false"));
}

COS_ASSUME_NONNULL_END
