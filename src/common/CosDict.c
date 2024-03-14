/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/common/CosDict.h"

#include "common/Assert.h"

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

const CosDictKeyCallbacks cos_dict_obj_key_callbacks = {
    .hash = NULL,
    .retain = NULL,
    .release = NULL,
    .equal = NULL,
};

const CosDictValueCallbacks cos_dict_obj_value_callbacks = {
    .retain = NULL,
    .release = NULL,
    .equal = NULL,
};

struct CosDict {
    size_t count;
    size_t capacity;

    CosDictKeyCallbacks *key_callbacks;
    CosDictValueCallbacks *value_callbacks;
};

CosDict *
cos_dict_alloc(const CosDictKeyCallbacks * COS_Nullable key_callbacks,
               const CosDictValueCallbacks * COS_Nullable value_callbacks,
               size_t capacity_hint)
{
    CosDict *dict;

    dict = calloc(1, sizeof(CosDict));
    if (!dict) {
        goto failure;
    }

    if (key_callbacks) {
        dict->key_callbacks = malloc(sizeof(CosDictKeyCallbacks));
        if (!dict->key_callbacks) {
            goto failure;
        }

        memcpy(dict->key_callbacks,
               key_callbacks,
               sizeof(CosDictKeyCallbacks));
    }

    if (value_callbacks) {
        dict->value_callbacks = malloc(sizeof(CosDictValueCallbacks));
        if (!dict->value_callbacks) {
            goto failure;
        }

        memcpy(dict->value_callbacks,
               value_callbacks,
               sizeof(CosDictValueCallbacks));
    }

    return dict;

failure:
    if (dict) {
        cos_dict_free(dict);
    }
    return NULL;
}

void
cos_dict_free(CosDict *dict)
{
    if (!dict) {
        return;
    }

    free(dict->key_callbacks);
    free(dict->value_callbacks);

    free(dict);
}

const CosDictKeyCallbacks *
cos_dict_get_key_callbacks(const CosDict *dict)
{
    COS_PARAMETER_ASSERT(dict != NULL);
    if (!dict) {
        return NULL;
    }

    return dict->key_callbacks;
}

const CosDictValueCallbacks *
cos_dict_get_value_callbacks(const CosDict *dict)
{
    COS_PARAMETER_ASSERT(dict != NULL);
    if (!dict) {
        return NULL;
    }

    return dict->value_callbacks;
}

size_t
cos_dict_get_count(const CosDict *dict)
{
    COS_PARAMETER_ASSERT(dict != NULL);
    if (!dict) {
        return 0;
    }

    return dict->count;
}

bool
cos_dict_set(CosDict *dict,
             void *key,
             void *value,
             CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(dict != NULL);
    COS_PARAMETER_ASSERT(key != NULL);
    COS_PARAMETER_ASSERT(value != NULL);
    if (!dict || !key || !value) {
        return false;
    }

    return true;
}

COS_ASSUME_NONNULL_END
