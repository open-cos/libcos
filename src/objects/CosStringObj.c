/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/objects/CosStringObj.h"

#include "common/Assert.h"
#include "libcos/objects/CosObj.h"

#include <libcos/common/CosData.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosStringObj {
    CosObjType type;

    CosData *data;
};

CosStringObj * COS_Nullable
cos_string_obj_alloc(CosData *data)
{
    COS_PARAMETER_ASSERT(data != NULL);
    if (!data) {
        return NULL;
    }

    CosStringObj * const string_obj = calloc(1, sizeof(CosStringObj));
    if (!string_obj) {
        return NULL;
    }

    string_obj->type = CosObjType_String;
    string_obj->data = data;

    return string_obj;
}

void
cos_string_obj_free(CosStringObj *string_obj)
{
    if (!string_obj) {
        return;
    }

    cos_data_free(string_obj->data);
    free(string_obj);
}

void
cos_string_obj_set_value(CosStringObj *string_obj,
                         CosData *data)
{
    COS_PARAMETER_ASSERT(string_obj != NULL);
    COS_PARAMETER_ASSERT(data != NULL);
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

CosData *
cos_string_obj_get_value(const CosStringObj *string_obj)
{
    COS_PARAMETER_ASSERT(string_obj != NULL);
    if (!string_obj) {
        return NULL;
    }

    return string_obj->data;
}

COS_ASSUME_NONNULL_END
