/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/objects/CosIntObjNode.h"

#include "common/Assert.h"

#include "libcos/objects/CosObjNode.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosIntObjNode {
    CosObjNodeType type;
    unsigned int ref_count;

    int value;
};

CosIntObjNode *
cos_int_obj_node_alloc(int value)
{
    CosIntObjNode * const int_obj = calloc(1, sizeof(CosIntObjNode));
    if (!int_obj) {
        return NULL;
    }

    int_obj->type = CosObjNodeType_Integer;
    int_obj->ref_count = 1;
    int_obj->value = value;

    return int_obj;
}

void
cos_int_obj_node_free(CosIntObjNode *int_obj)
{
    if (!int_obj) {
        return;
    }

    free(int_obj);
}

int
cos_int_obj_node_get_value(const CosIntObjNode *int_obj)
{
    COS_API_PARAM_CHECK(int_obj != NULL);
    if (!int_obj) {
        return 0;
    }

    return int_obj->value;
}

void
cos_int_obj_node_set_value(CosIntObjNode *int_obj,
                      int value)
{
    COS_API_PARAM_CHECK(int_obj != NULL);
    if (!int_obj) {
        return;
    }

    int_obj->value = value;
}

void
cos_int_obj_node_print_desc(const CosIntObjNode *int_obj)
{
    COS_API_PARAM_CHECK(int_obj != NULL);
    if (!int_obj) {
        return;
    }

    printf("Integer: %d\n", int_obj->value);
}

COS_ASSUME_NONNULL_END
