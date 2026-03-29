/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/objects/CosDictObjNode.h"

#include "common/Assert.h"

#include "libcos/common/CosDict.h"
#include "libcos/objects/CosNameObjNode.h"
#include "libcos/objects/CosObjNode.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosDictObjNode {
    CosObjNodeType type;
    unsigned int ref_count;

    CosDict *value;
};

static size_t
cos_dict_obj_node_key_hash_(void *key)
{
    COS_PARAM_ASSERT_INTERNAL(key != NULL);

    // The key is a pointer to a name object (pointer).
    CosNameObjNode * const name_obj = (CosNameObjNode *)key;
    COS_ASSERT(name_obj != NULL, "Expected a name object");

    return cos_name_obj_node_get_hash(name_obj);
}

static void
cos_dict_obj_node_key_value_release_(void *key)
{
    COS_IMPL_PARAM_CHECK(key != NULL);

    CosObjNode * const obj = (CosObjNode *)key;
    COS_ASSERT(obj != NULL, "Expected an object");

    cos_obj_node_free(obj);
}

static bool
cos_dict_obj_node_keys_equal_(void *key1,
                         void *key2)
{
    COS_PARAM_ASSERT_INTERNAL(key1 != NULL);
    COS_PARAM_ASSERT_INTERNAL(key2 != NULL);

    CosNameObjNode * const name_obj1 = (CosNameObjNode *)key1;
    CosNameObjNode * const name_obj2 = (CosNameObjNode *)key2;
    COS_ASSERT(name_obj1 != NULL, "Expected a first name object");
    COS_ASSERT(name_obj2 != NULL, "Expected a second name object");

    if (name_obj1 == name_obj2) {
        return true;
    }

    return cos_name_obj_node_equal(name_obj1,
                              name_obj2);
}

const CosDictKeyCallbacks cos_dict_obj_node_key_callbacks = {
    .hash = &cos_dict_obj_node_key_hash_,
    .retain = NULL,
    .release = &cos_dict_obj_node_key_value_release_,
    .equal = &cos_dict_obj_node_keys_equal_,
};

const CosDictValueCallbacks cos_dict_obj_node_value_callbacks = {
    .retain = NULL,
    .release = &cos_dict_obj_node_key_value_release_,
    .equal = NULL,
};

CosDictObjNode *
cos_dict_obj_node_create(CosDict * COS_Nullable dict)
{
    CosDictObjNode * const dict_obj = calloc(1, sizeof(CosDictObjNode));
    if (!dict_obj) {
        goto failure;
    }

    dict_obj->type = CosObjNodeType_Dict;
    dict_obj->ref_count = 1;

    if (dict) {
        dict_obj->value = COS_nonnull_cast(dict);
    }
    else {
        CosDict * const new_dict = cos_dict_create(&cos_dict_obj_node_key_callbacks,
                                                   &cos_dict_obj_node_value_callbacks,
                                                   0);
        if (!new_dict) {
            goto failure;
        }
        dict_obj->value = new_dict;
    }
    COS_ASSERT(dict_obj->value != NULL, "Expected a dictionary value");

    return dict_obj;

failure:
    if (dict_obj) {
        free(dict_obj);
    }
    return NULL;
}

void
cos_dict_obj_node_destroy(CosDictObjNode *dict_obj)
{
    if (!dict_obj) {
        return;
    }

    cos_dict_destroy(dict_obj->value);
    free(dict_obj);
}

size_t
cos_dict_obj_node_get_count(const CosDictObjNode *dict_obj)
{
    COS_API_PARAM_CHECK(dict_obj != NULL);
    if (!dict_obj) {
        return 0;
    }

    return cos_dict_get_count(dict_obj->value);
}

bool
cos_dict_obj_node_get_value(const CosDictObjNode *dict_obj,
                       CosNameObjNode *key,
                       CosObjNode * COS_Nullable *out_value,
                       CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(dict_obj != NULL);
    COS_API_PARAM_CHECK(key != NULL);
    COS_API_PARAM_CHECK(out_value != NULL);
    if (!dict_obj || !key || !out_value) {
        return false;
    }

    return cos_dict_get(dict_obj->value,
                        key,
                        (void **)out_value,
                        out_error);
}

bool
cos_dict_obj_node_get_value_with_string(const CosDictObjNode *dict_obj,
                                   const char *key,
                                   CosObjNode * COS_Nullable *out_value,
                                   CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(dict_obj != NULL);
    COS_API_PARAM_CHECK(key != NULL);
    COS_API_PARAM_CHECK(out_value != NULL);
    if (!dict_obj || !key || !out_value) {
        return false;
    }

    CosString *key_str = NULL;
    CosNameObjNode *name_obj = NULL;

    key_str = cos_string_alloc_with_str(key);
    if (!key_str) {
        goto failure;
    }

    name_obj = cos_name_obj_node_alloc(key_str);
    if (!name_obj) {
        goto failure;
    }

    const bool result = cos_dict_get(dict_obj->value,
                                     name_obj,
                                     (void **)out_value,
                                     out_error);
    cos_name_obj_node_free(name_obj);

    return result;

failure:
    if (key_str) {
        cos_string_free(key_str);
    }
    if (name_obj) {
        cos_name_obj_node_free(name_obj);
    }

    return false;
}

bool
cos_dict_obj_node_set(CosDictObjNode *dict_obj,
                 CosNameObjNode *key,
                 CosObjNode *value,
                 CosError * COS_Nullable error)
{
    COS_API_PARAM_CHECK(dict_obj != NULL);
    COS_API_PARAM_CHECK(key != NULL);
    COS_API_PARAM_CHECK(value != NULL);
    if (!dict_obj || !key || !value) {
        return false;
    }

    return cos_dict_set(dict_obj->value,
                        key,
                        value,
                        error);
}

// MARK: - Iterator

CosDictObjNodeIterator
cos_dict_obj_node_iterator_init(CosDictObjNode *dict_obj)
{
    COS_API_PARAM_CHECK(dict_obj != NULL);

    CosDictObjNodeIterator iterator = {
        .base = cos_dict_iterator_init(dict_obj->value),
    };
    return iterator;
}

bool
cos_dict_obj_node_iterator_next(CosDictObjNodeIterator *iterator,
                           CosNameObjNode * COS_Nullable * COS_Nonnull out_key,
                           CosObjNode * COS_Nullable * COS_Nonnull out_value)
{
    COS_API_PARAM_CHECK(iterator != NULL);
    COS_API_PARAM_CHECK(out_key != NULL);
    COS_API_PARAM_CHECK(out_value != NULL);
    if (COS_UNLIKELY(!iterator || !out_key || !out_value)) {
        return false;
    }

    return cos_dict_iterator_next(&iterator->base,
                                  (void **)out_key,
                                  (void **)out_value);
}

COS_ASSUME_NONNULL_END
