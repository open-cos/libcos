/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/common/CosDict.h"

#include <stdlib.h>
#include <string.h>

#include "Assert.h"

COS_ASSUME_NONNULL_BEGIN

struct CosDict {
    size_t count;
    size_t capacity;

    CosDictKeyCallbacks *key_callbacks;
    CosDictValueCallbacks *value_callbacks;
};

CosDict *
cos_dict_alloc(size_t capacity,
               const CosDictKeyCallbacks * COS_Nullable key_callbacks,
               const CosDictValueCallbacks * COS_Nullable value_callbacks)
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

COS_ASSUME_NONNULL_END
