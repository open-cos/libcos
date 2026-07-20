//
// Created by david on 15/10/23.
//

#include "libcos/objects/CosArrayObjNode.h"

#include "common/Assert.h"

#include "libcos/objects/CosObjNode.h"

#include <libcos/common/CosArray.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

static void
cos_array_obj_node_callbacks_release_(void *item);

static bool
cos_array_obj_node_callbacks_equal_(const void *item1,
                               const void *item2);

const CosArrayCallbacks cos_array_obj_node_callbacks = {
    .retain = NULL,
    .release = &cos_array_obj_node_callbacks_release_,
    .equal = &cos_array_obj_node_callbacks_equal_,
};

struct CosArrayObjNode {
    CosObjNodeType type;
    unsigned int ref_count;

    CosArray *value;
};

CosArrayObjNode *
cos_array_obj_node_alloc(CosArray * COS_Nullable array)
{
    CosArrayObjNode * const array_obj = calloc(1, sizeof(CosArrayObjNode));
    if (!array_obj) {
        goto failure;
    }

    array_obj->type = CosObjNodeType_Array;
    array_obj->ref_count = 1;

    if (array) {
        array_obj->value = COS_nonnull_cast(array);
    }
    else {
        CosArray * const new_array = cos_array_create(sizeof(CosObjNode *),
                                                      &cos_array_obj_node_callbacks,
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
cos_array_obj_node_free(CosArrayObjNode *array_obj)
{
    if (!array_obj) {
        return;
    }

    cos_array_destroy(array_obj->value);
    free(array_obj);
}

size_t
cos_array_obj_node_get_count(const CosArrayObjNode *array_obj)
{
    COS_API_PARAM_CHECK(array_obj != NULL);
    if (!array_obj) {
        return 0;
    }

    return cos_array_get_count(array_obj->value);
}

CosObjNode *
cos_array_obj_node_get_at(const CosArrayObjNode *array_obj,
                     size_t index,
                     CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(array_obj != NULL);
    if (!array_obj) {
        return NULL;
    }

    CosObjNode *obj = NULL;
    if (!cos_array_get_item(array_obj->value,
                            index,
                            (void *)&obj,
                            out_error)) {
        return NULL;
    }
    return obj;
}

bool
cos_array_obj_node_insert(CosArrayObjNode *array_obj,
                     size_t index,
                     CosObjNode *obj,
                     CosError * COS_Nullable error)
{
    COS_API_PARAM_CHECK(array_obj != NULL);
    COS_API_PARAM_CHECK(obj != NULL);
    if (!array_obj || !obj) {
        return false;
    }

    return cos_array_insert_item(array_obj->value,
                                 index,
                                 (void *)&obj,
                                 error);
}

bool
cos_array_obj_node_append(CosArrayObjNode *array_obj,
                     CosObjNode *obj,
                     CosError * COS_Nullable error)
{
    COS_API_PARAM_CHECK(array_obj != NULL);
    COS_API_PARAM_CHECK(obj != NULL);
    if (!array_obj || !obj) {
        return false;
    }

    return cos_array_append_item(array_obj->value,
                                 (void *)&obj,
                                 error);
}

bool
cos_array_obj_node_remove_at(CosArrayObjNode *array_obj,
                        size_t index,
                        CosError * COS_Nullable error)
{
    COS_API_PARAM_CHECK(array_obj != NULL);
    if (!array_obj) {
        return false;
    }

    return cos_array_remove_item(array_obj->value,
                                 index,
                                 error);
}

static void
cos_array_obj_node_callbacks_release_(void *item)
{
    COS_ASSERT(item != NULL, "Expected a non-NULL item");
    if (!item) {
        return;
    }

    CosObjNode * const obj = *(CosObjNode **)item;
    cos_obj_node_release(obj);
}

static bool
cos_array_obj_node_callbacks_equal_(const void *item1,
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

    const CosObjNode * const obj1 = *(CosObjNode * const *)item1;
    const CosObjNode * const obj2 = *(CosObjNode * const *)item2;

    return obj1 == obj2;
}

COS_ASSUME_NONNULL_END
