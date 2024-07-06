/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/objects/CosDictObj.h"

#include "common/Assert.h"

#include "libcos/common/CosDict.h"
#include "libcos/objects/CosNameObj.h"
#include "libcos/objects/CosObj.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosDictObj {
    CosObjType type;

    CosDict *value;
};

static size_t
cos_dict_obj_key_hash_(void *key)
{
    COS_PARAM_ASSERT_INTERNAL(key != NULL);

    // The key is a pointer to a name object (pointer).
    CosNameObj * const name_obj = (CosNameObj *)key;
    COS_ASSERT(name_obj != NULL, "Expected a name object");

    return cos_name_obj_get_hash(name_obj);
}

static bool
cos_dict_obj_keys_equal_(void *key1,
                         void *key2)
{
    COS_PARAM_ASSERT_INTERNAL(key1 != NULL);
    COS_PARAM_ASSERT_INTERNAL(key2 != NULL);

    CosNameObj * const name_obj1 = (CosNameObj *)key1;
    CosNameObj * const name_obj2 = (CosNameObj *)key2;
    COS_ASSERT(name_obj1 != NULL, "Expected a first name object");
    COS_ASSERT(name_obj2 != NULL, "Expected a second name object");

    if (name_obj1 == name_obj2) {
        return true;
    }

    return cos_name_obj_equal(name_obj1,
                              name_obj2);
}

const CosDictKeyCallbacks cos_dict_obj_key_callbacks = {
    .hash = &cos_dict_obj_key_hash_,
    .retain = NULL,
    .release = NULL,
    .equal = &cos_dict_obj_keys_equal_,
};

const CosDictValueCallbacks cos_dict_obj_value_callbacks = {
    .retain = NULL,
    .release = NULL,
    .equal = NULL,
};

CosDictObj *
cos_dict_obj_create(CosDict * COS_Nullable dict)
{
    CosDictObj * const dict_obj = calloc(1, sizeof(CosDictObj));
    if (!dict_obj) {
        goto failure;
    }

    dict_obj->type = CosObjType_Dict;

    if (dict) {
        dict_obj->value = COS_nonnull_cast(dict);
    }
    else {
        CosDict * const new_dict = cos_dict_create(&cos_dict_obj_key_callbacks,
                                                   &cos_dict_obj_value_callbacks,
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
cos_dict_obj_destroy(CosDictObj *dict_obj)
{
    if (!dict_obj) {
        return;
    }

    cos_dict_destroy(dict_obj->value);
    free(dict_obj);
}

size_t
cos_dict_obj_get_count(const CosDictObj *dict_obj)
{
    COS_PARAMETER_ASSERT(dict_obj != NULL);
    if (!dict_obj) {
        return 0;
    }

    return cos_dict_get_count(dict_obj->value);
}

bool
cos_dict_obj_get_value(const CosDictObj *dict_obj,
                       CosNameObj *key,
                       CosObj * COS_Nullable *out_value,
                       CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(dict_obj != NULL);
    COS_PARAMETER_ASSERT(key != NULL);
    COS_PARAMETER_ASSERT(out_value != NULL);
    if (!dict_obj || !key || !out_value) {
        return false;
    }

    return cos_dict_get(dict_obj->value,
                        key,
                        (void **)out_value,
                        out_error);
}

bool
cos_dict_obj_get_value_with_string(const CosDictObj *dict_obj,
                                   const char *key,
                                   CosObj * COS_Nullable *out_value,
                                   CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(dict_obj != NULL);
    COS_PARAMETER_ASSERT(key != NULL);
    COS_PARAMETER_ASSERT(out_value != NULL);
    if (!dict_obj || !key || !out_value) {
        return false;
    }

    CosString *key_str = NULL;
    CosNameObj *name_obj = NULL;

    key_str = cos_string_alloc_with_str(key);
    if (!key_str) {
        goto failure;
    }

    name_obj = cos_name_obj_alloc(key_str);
    if (!name_obj) {
        goto failure;
    }

    const bool result = cos_dict_get(dict_obj->value,
                                     name_obj,
                                     (void **)out_value,
                                     out_error);
    cos_name_obj_free(name_obj);

    return result;

failure:
    if (key_str) {
        cos_string_free(key_str);
    }
    if (name_obj) {
        cos_name_obj_free(name_obj);
    }

    return false;
}

bool
cos_dict_obj_set(CosDictObj *dict_obj,
                 CosNameObj *key,
                 CosObj *value,
                 CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(dict_obj != NULL);
    COS_PARAMETER_ASSERT(key != NULL);
    COS_PARAMETER_ASSERT(value != NULL);
    if (!dict_obj || !key || !value) {
        return false;
    }

    return cos_dict_set(dict_obj->value,
                        key,
                        value,
                        error);
}

COS_ASSUME_NONNULL_END
