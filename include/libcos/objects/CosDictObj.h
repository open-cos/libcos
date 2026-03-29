/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COS_DICT_OBJ_H
#define LIBCOS_COS_DICT_OBJ_H

#include "common/CosDict.h"

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosString.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief Destroys a dictionary object.
 *
 * @param dict_obj The dictionary object.
 */
void
cos_dict_obj_destroy(CosDictObj *dict_obj)
    COS_DEALLOCATOR_FUNC;

/**
 * @brief Creates a dictionary object.
 *
 * @param dict The dictionary or @c NULL.
 *
 * @return The dictionary object, or @c NULL if an error occurred.
 */
CosDictObj * COS_Nullable
cos_dict_obj_create(CosDict * COS_Nullable dict)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_dict_obj_destroy);

/**
 * @brief Gets the number of entries in a dictionary object.
 *
 * @param dict_obj The dictionary object.
 *
 * @return The number of entries in the dictionary object.
 */
size_t
cos_dict_obj_get_count(const CosDictObj *dict_obj);

/**
 * @brief Gets the object value for a given key in a dictionary object.
 *
 * @param dict_obj The dictionary object.
 * @param key The key.
 * @param out_value The output parameter for the object value.
 * @param out_error The error information.
 *
 * @return @c true if the object value was found, otherwise @c false.
 */
bool
cos_dict_obj_get_value(const CosDictObj *dict_obj,
                       CosNameObj *key,
                       CosObj * COS_Nullable * COS_Nonnull out_value,
                       CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(3)
    COS_ATTR_ACCESS_WRITE_ONLY(4);

/**
 * @brief Gets the object value for a given key in a dictionary object.
 *
 * @param dict_obj The dictionary object.
 * @param key The key, as a nul-terminated string.
 * @param out_value The output parameter for the object value.
 * @param out_error The error information.
 *
 * @return @c true if the object value was found, otherwise @c false.
 */
bool
cos_dict_obj_get_value_with_string(const CosDictObj *dict_obj,
                                   const char *key,
                                   CosObj * COS_Nullable * COS_Nonnull out_value,
                                   CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(3)
    COS_ATTR_ACCESS_WRITE_ONLY(4);

bool
cos_dict_obj_set(CosDictObj *dict_obj,
                 CosNameObj *key,
                 CosObj *value,
                 CosError * COS_Nullable error);

extern const CosDictKeyCallbacks cos_dict_obj_key_callbacks;
extern const CosDictValueCallbacks cos_dict_obj_value_callbacks;

// MARK: - Iterator

/**
 * An iterator over the key-value pairs in a dictionary object.
 *
 * Initialize with @c cos_dict_obj_iterator_init and advance with
 * @c cos_dict_obj_iterator_next.  The iterator does not own any resources
 * and becomes invalid if the dictionary is mutated.
 */
typedef struct CosDictObjIterator {
    /**
     * The underlying dictionary iterator.
     * @private
     */
    CosDictIterator base;
} CosDictObjIterator;

/**
 * Initializes a dictionary object iterator.
 *
 * @param dict_obj The dictionary object to iterate over.
 *
 * @return An iterator positioned before the first entry.
 */
CosDictObjIterator
cos_dict_obj_iterator_init(CosDictObj *dict_obj);

/**
 * Advances the iterator to the next key-value pair.
 *
 * @param iterator The iterator.
 * @param[out] out_key On success, receives the key (a name object).
 * @param[out] out_value On success, receives the value.
 *
 * @return @c true if another entry was found, @c false when iteration is
 * complete.
 */
bool
cos_dict_obj_iterator_next(CosDictObjIterator *iterator,
                           CosNameObj * COS_Nullable * COS_Nonnull out_key,
                           CosObj * COS_Nullable * COS_Nonnull out_value)
    COS_ATTR_ACCESS_WRITE_ONLY(2)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_DICT_OBJ_H */
