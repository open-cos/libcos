/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_DICT_H
#define LIBCOS_COMMON_COS_DICT_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

// MARK: - Callbacks

typedef size_t (*CosDictHashCallback)(void *value);
typedef void (*CosDictRetainCallback)(void *value);
typedef void (*CosDictReleaseCallback)(void *value);
typedef bool (*CosDictEqualValuesCallback)(void *value1,
                                           void *value2);

typedef struct CosDictKeyCallbacks {
    CosDictHashCallback hash;
    CosDictRetainCallback COS_Nullable retain;
    CosDictReleaseCallback COS_Nullable release;
    CosDictEqualValuesCallback equal;
} CosDictKeyCallbacks;

typedef struct CosDictValueCallbacks {
    CosDictRetainCallback COS_Nullable retain;
    CosDictReleaseCallback COS_Nullable release;
    CosDictEqualValuesCallback COS_Nullable equal;
} CosDictValueCallbacks;

void
cos_dict_destroy(CosDict *dict)
    COS_DEALLOCATOR_FUNC;

CosDict * COS_Nullable
cos_dict_create(const CosDictKeyCallbacks *key_callbacks,
                const CosDictValueCallbacks *value_callbacks,
                size_t capacity_hint)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_dict_destroy);

size_t
cos_dict_get_count(const CosDict *dict);

bool
cos_dict_get(CosDict *dict,
             void *key,
             void * COS_Nullable * COS_Nonnull out_value,
             CosError * COS_Nullable out_error);

bool
cos_dict_set(CosDict *dict,
             void *key,
             void *value,
             CosError * COS_Nullable error);

// MARK: - Iterator

/**
 * An iterator over the key-value pairs in a dictionary.
 *
 * Initialize with @c cos_dict_iterator_init and advance with
 * @c cos_dict_iterator_next.  The iterator does not own any resources and
 * becomes invalid if the dictionary is mutated.
 */
typedef struct CosDictIterator {
    /**
     * The dictionary being iterated.
     * @private
     */
    CosDict *dict;

    /**
     * The current internal slot index.
     * @private
     */
    size_t index;
} CosDictIterator;

/**
 * Initializes a dictionary iterator.
 *
 * @param dict The dictionary to iterate over.
 *
 * @return An iterator positioned before the first entry.
 */
CosDictIterator
cos_dict_iterator_init(CosDict *dict);

/**
 * Advances the iterator to the next key-value pair.
 *
 * @param iterator The iterator.
 * @param[out] out_key On success, receives the key.
 * @param[out] out_value On success, receives the value.
 *
 * @return @c true if another entry was found, @c false when iteration is
 * complete.
 */
bool
cos_dict_iterator_next(CosDictIterator *iterator,
                       void * COS_Nullable * COS_Nonnull out_key,
                       void * COS_Nullable * COS_Nonnull out_value);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_DICT_H */
