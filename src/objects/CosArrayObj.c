//
// Created by david on 15/10/23.
//

#include "libcos/objects/CosArrayObj.h"

#include "common/Assert.h"
#include "libcos/objects/CosObj.h"

#include <libcos/common/CosArray.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

const CosArrayCallbacks cos_array_obj_callbacks = {
    .retain = NULL,
    .release = NULL,
    .equal = NULL,
};

struct CosArrayObj {
    CosObjType type;

    CosArray *value;
};

CosArrayObj *
cos_array_obj_alloc(CosArray * COS_Nullable array)
{
    CosArrayObj * const array_obj = calloc(1, sizeof(CosArrayObj));
    if (!array_obj) {
        goto failure;
    }

    array_obj->type = CosObjType_Array;

    if (array) {
        array_obj->value = (CosArray *)array;
    }
    else {
        CosArray * const new_array = cos_array_alloc(sizeof(CosObj *),
                                                     &cos_array_obj_callbacks,
                                                     0);
        if (!new_array) {
            goto failure;
        }
        array_obj->value = new_array;
    }
    COS_ASSERT(array_obj->value != NULL, "Expected an array value");

    return array_obj;

failure:
    if (array_obj) {
        free(array_obj);
    }
    return NULL;
}

void
cos_array_obj_free(CosArrayObj *array_obj)
{
    if (!array_obj) {
        return;
    }

    const size_t count = cos_array_get_count(array_obj->value);
    for (size_t i = 0; i < count; i++) {
        CosObj * const obj = cos_array_get_item(array_obj->value, i, NULL);
        cos_obj_free(obj);
    }
    cos_array_free(array_obj->value);
    free(array_obj);
}

size_t
cos_array_obj_get_count(const CosArrayObj *array_obj)
{
    COS_PARAMETER_ASSERT(array_obj != NULL);
    if (!array_obj) {
        return 0;
    }

    return cos_array_get_count(array_obj->value);
}

CosObj *
cos_array_obj_get_at(const CosArrayObj *array_obj,
                     size_t index,
                     CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(array_obj != NULL);
    if (!array_obj) {
        return NULL;
    }

    return cos_array_get_item(array_obj->value,
                              index,
                              out_error);
}

bool
cos_array_obj_insert(CosArrayObj *array_obj,
                     size_t index,
                     CosObj *obj,
                     CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(array_obj != NULL);
    COS_PARAMETER_ASSERT(obj != NULL);
    if (!array_obj || !obj) {
        return false;
    }

    return cos_array_insert_item(array_obj->value,
                                 index,
                                 &obj,
                                 error);
}

bool
cos_array_obj_append(CosArrayObj *array_obj,
                     CosObj *obj,
                     CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(array_obj != NULL);
    COS_PARAMETER_ASSERT(obj != NULL);
    if (!array_obj || !obj) {
        return false;
    }

    return cos_array_append_item(array_obj->value,
                                 &obj,
                                 error);
}

COS_ASSUME_NONNULL_END
