//
// Created by david on 15/10/23.
//

#include "libcos/objects/CosArrayObj.h"

#include "common/Assert.h"
#include "libcos/objects/CosObj.h"

#include <libcos/common/CosArray.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

static void
cos_array_obj_callbacks_release_(void *item);

static bool
cos_array_obj_callbacks_equal_(const void *item1,
                               const void *item2);

const CosArrayCallbacks cos_array_obj_callbacks = {
    .retain = NULL,
    .release = &cos_array_obj_callbacks_release_,
    .equal = &cos_array_obj_callbacks_equal_,
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
        CosArray * const new_array = cos_array_create(sizeof(CosObj *),
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

    cos_array_destroy(array_obj->value);
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

    CosObj *obj = NULL;
    if (!cos_array_get_item(array_obj->value,
                            index,
                            &obj,
                            out_error)) {
        return NULL;
    }
    return obj;
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

bool
cos_array_obj_remove_at(CosArrayObj *array_obj,
                        size_t index,
                        CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(array_obj != NULL);
    if (!array_obj) {
        return false;
    }

    return cos_array_remove_item(array_obj->value,
                                 index,
                                 error);
}

static void
cos_array_obj_callbacks_release_(void *item)
{
    COS_ASSERT(item != NULL, "Expected a non-NULL item");
    if (!item) {
        return;
    }

    CosObj * const obj = *(CosObj **)item;
    cos_obj_free(obj);
}

static bool
cos_array_obj_callbacks_equal_(const void *item1,
                               const void *item2)
{
    COS_ASSERT(item1 != NULL, "Expected a non-NULL item1");
    COS_ASSERT(item2 != NULL, "Expected a non-NULL item2");
    if (item1 == item2) {
        return true;
    }
    else if (!item1 || !item2) {
        return false;
    }

    const CosObj * const obj1 = *(CosObj * const *)item1;
    const CosObj * const obj2 = *(CosObj * const *)item2;

    return obj1 == obj2;
}

COS_ASSUME_NONNULL_END
