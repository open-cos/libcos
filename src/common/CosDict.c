/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/common/CosDict.h"

#include "common/Assert.h"
#include "common/CosContainerUtils.h"

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

typedef struct CosDictEntry {
    void * COS_Nullable key;
    void * COS_Nullable value;
} CosDictEntry;

struct CosDict {
    CosDictEntry *entries;
    size_t capacity;

    size_t count;

    CosDictKeyCallbacks key_callbacks;
    CosDictValueCallbacks value_callbacks;
};

COS_STATIC_INLINE
size_t
cos_dict_hash_(const CosDict *dict,
               void *key);

COS_STATIC_INLINE
bool
cos_dict_grow_(CosDict *dict,
               size_t new_capacity);

COS_STATIC_INLINE
CosDictEntry *
cos_dict_find_entry_(const CosDict *dict,
                     void *key,
                     size_t hash);

CosDict *
cos_dict_create(const CosDictKeyCallbacks *key_callbacks,
                const CosDictValueCallbacks *value_callbacks,
                size_t capacity_hint)
{
    COS_PARAMETER_ASSERT(key_callbacks != NULL);
    COS_PARAMETER_ASSERT(value_callbacks != NULL);
    if (COS_UNLIKELY(!key_callbacks || !value_callbacks)) {
        return NULL;
    }

    CosDict *dict = NULL;
    CosDictEntry *entries = NULL;

    dict = calloc(1, sizeof(CosDict));
    if (!dict) {
        goto failure;
    }

    dict->key_callbacks = *key_callbacks;
    dict->value_callbacks = *value_callbacks;

    const size_t capacity = cos_container_round_capacity_(capacity_hint);

    entries = calloc(capacity, sizeof(CosDictEntry));
    if (!entries) {
        goto failure;
    }

    dict->entries = entries;
    dict->capacity = capacity;

    return dict;

failure:
    if (dict) {
        cos_dict_destroy(dict);
    }
    if (entries) {
        free(entries);
    }
    return NULL;
}

void
cos_dict_destroy(CosDict *dict)
{
    if (!dict) {
        return;
    }

    free(dict->entries);

    free(dict);
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
cos_dict_get(CosDict *dict,
             void *key,
             void * COS_Nullable * COS_Nonnull out_value,
             CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(dict != NULL);
    COS_PARAMETER_ASSERT(key != NULL);
    COS_PARAMETER_ASSERT(out_value != NULL);
    if (COS_UNLIKELY(!dict || !key || !out_value)) {
        return false;
    }

    const size_t hash = cos_dict_hash_(dict, key);

    CosDictEntry * const entry = cos_dict_find_entry_(dict, key, hash);
    if (!entry || entry->key == NULL) {
        return false;
    }

    *out_value = entry->value;

    return true;
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

    if (dict->count >= (dict->capacity / 4) * 3) {
        const size_t new_capacity = cos_container_round_capacity_(dict->capacity + 1);
        if (!cos_dict_grow_(dict,
                            new_capacity)) {
            return false;
        }
    }

    const size_t hash = cos_dict_hash_(dict, key);

    CosDictEntry * const entry = cos_dict_find_entry_(dict, key, hash);
    if (!entry) {
        return false;
    }

    const bool is_new = (entry->key == NULL);
    if (is_new) {
        dict->count++;
    }

    const CosDictReleaseCallback retain_key = dict->key_callbacks.retain;
    if (retain_key) {
        retain_key(key);
    }
    const CosDictRetainCallback retain_value = dict->value_callbacks.retain;
    if (retain_value) {
        retain_value(value);
    }

    entry->key = key;
    entry->value = value;

    return true;
}

COS_STATIC_INLINE
bool
cos_dict_grow_(CosDict *dict,
               size_t new_capacity)
{
    COS_PARAM_ASSERT_INTERNAL(dict != NULL);
    COS_PARAM_ASSERT_INTERNAL(new_capacity > dict->capacity);

    CosDictEntry *new_entries = NULL;

    new_entries = calloc(new_capacity, sizeof(CosDictEntry));
    if (COS_UNLIKELY(!new_entries)) {
        goto failure;
    }

    CosDictEntry * const old_entries = dict->entries;
    const size_t old_capacity = dict->capacity;

    dict->entries = new_entries;
    dict->capacity = new_capacity;

    // Copy all the old entries to the new array.
    for (size_t i = 0; i < old_capacity; i++) {
        CosDictEntry * const old_entry = &old_entries[i];

        if (old_entry->key == NULL) {
            // Empty entry - skip.
            continue;
        }

        const size_t hash = cos_dict_hash_(dict, COS_nonnull_cast(old_entry->key));

        CosDictEntry * const new_entry = cos_dict_find_entry_(dict,
                                                              COS_nonnull_cast(old_entry->key),
                                                              hash);
        new_entry->key = old_entry->key;
        new_entry->value = old_entry->value;
    }

    free(old_entries);

    return true;

failure:
    if (new_entries) {
        free(new_entries);
    }
    return false;
}

COS_STATIC_INLINE
CosDictEntry *
cos_dict_find_entry_(const CosDict *dict,
                     void *key,
                     size_t hash)
{
    COS_PARAM_ASSERT_INTERNAL(dict != NULL);
    COS_PARAM_ASSERT_INTERNAL(key != NULL);

    const size_t capacity = dict->capacity;
    const CosDictEqualValuesCallback key_equal = dict->key_callbacks.equal;

    size_t index = hash % capacity;
    while (true) {
        CosDictEntry * const entry = &dict->entries[index];

        if (entry->key == NULL || key_equal(COS_nonnull_cast(entry->key),
                                            key)) {
            return entry;
        }

        index = (index + 1) % capacity;
    }
}

COS_STATIC_INLINE
size_t
cos_dict_hash_(const CosDict *dict,
               void *key)
{
    COS_PARAM_ASSERT_INTERNAL(dict != NULL);
    COS_PARAM_ASSERT_INTERNAL(key != NULL);

    if (dict->key_callbacks.hash) {
        return dict->key_callbacks.hash(key);
    }

    return (size_t)key;
}

COS_ASSUME_NONNULL_END
