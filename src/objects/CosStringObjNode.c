/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/objects/CosStringObjNode.h"

#include "common/Assert.h"

#include <libcos/common/CosData.h>
#include <libcos/objects/CosObjNode.h>

#include <stdio.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosStringObjNode {
    CosObjNodeType type;
    unsigned int ref_count;

    CosData *data;
};

CosStringObjNode * COS_Nullable
cos_string_obj_node_alloc(CosData *data)
{
    COS_API_PARAM_CHECK(data != NULL);
    if (!data) {
        return NULL;
    }

    CosStringObjNode * const string_obj = calloc(1, sizeof(CosStringObjNode));
    if (!string_obj) {
        return NULL;
    }

    string_obj->type = CosObjNodeType_String;
    string_obj->ref_count = 1;
    string_obj->data = data;

    return string_obj;
}

void
cos_string_obj_node_free(CosStringObjNode *string_obj)
{
    if (!string_obj) {
        return;
    }

    cos_data_free(string_obj->data);
    free(string_obj);
}

void
cos_string_obj_node_set_value(CosStringObjNode *string_obj,
                         CosData *data)
{
    COS_API_PARAM_CHECK(string_obj != NULL);
    COS_API_PARAM_CHECK(data != NULL);
    if (!string_obj || !data) {
        return;
    }

    if (string_obj->data == data) {
        // No change.
        return;
    }

    // Free the old data.
    cos_data_free(string_obj->data);

    string_obj->data = data;
}

const CosData *
cos_string_obj_node_get_value(const CosStringObjNode *string_obj)
{
    COS_API_PARAM_CHECK(string_obj != NULL);
    if (!string_obj) {
        return NULL;
    }

    return string_obj->data;
}

void
cos_string_obj_node_print_desc(const CosStringObjNode *string_obj)
{
    COS_API_PARAM_CHECK(string_obj != NULL);
    if (!string_obj) {
        return;
    }

    printf("String: \"%.*s\"\n",
           (int)(string_obj->data->size),
           string_obj->data->bytes);
}

COS_ASSUME_NONNULL_END
