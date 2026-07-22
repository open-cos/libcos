/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COS_DICT_OBJ_H
#define LIBCOS_COS_DICT_OBJ_H

#include "common/CosDict.h"

#include <libcos/common/CosAPI.h>
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
COS_API void
cos_dict_obj_node_destroy(CosDictObjNode *dict_obj)
    COS_DEALLOCATOR_FUNC;

/**
 * @brief Creates a dictionary object.
 *
 * @param dict The dictionary or @c NULL.
 *
 * @return The dictionary object, or @c NULL if an error occurred.
 */
COS_API CosDictObjNode * COS_Nullable
cos_dict_obj_node_create(CosDict * COS_Nullable dict)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_dict_obj_node_destroy);

/**
 * @brief Gets the number of entries in a dictionary object.
 *
 * @param dict_obj The dictionary object.
 *
 * @return The number of entries in the dictionary object.
 */
COS_API size_t
cos_dict_obj_node_get_count(const CosDictObjNode *dict_obj);

/**
 * @brief Looks up the object value for a given key in a dictionary object.
 *
 * A pure lookup that cannot fail, so it has no error channel.
 *
 * @param dict_obj The dictionary object.
 * @param key The key.
 * @param[out] out_value Receives the value if the key is present, or @c NULL if
 * it is absent. Always written.
 *
 * @return @c true if the key was present, @c false if it was absent.
 */
COS_API bool
cos_dict_obj_node_get_value(const CosDictObjNode *dict_obj,
                       CosNameObjNode *key,
                       CosObjNode * COS_Nullable * COS_Nonnull out_value)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

/**
 * @brief Looks up the object value for a given string key in a dictionary object.
 *
 * Allocating the temporary key node can fail, so a genuine absence and a real
 * failure are reported distinctly: an absent key is a @b success (returns
 * @c true with @c *out_value set to @c NULL), while an allocation failure
 * returns @c false with @p out_error set.
 *
 * @param dict_obj The dictionary object.
 * @param key The key, as a nul-terminated string.
 * @param[out] out_value On success, receives the value, or @c NULL if the key
 * is absent.
 * @param out_error On failure, receives the error information.
 *
 * @return @c true on success (whether or not the key was present), @c false if
 * the lookup could not be performed.
 */
COS_API bool
cos_dict_obj_node_get_value_with_string(const CosDictObjNode *dict_obj,
                                   const char *key,
                                   CosObjNode * COS_Nullable * COS_Nonnull out_value,
                                   CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(3)
    COS_ATTR_ACCESS_WRITE_ONLY(4);

COS_API bool
cos_dict_obj_node_set(CosDictObjNode *dict_obj,
                 CosNameObjNode *key,
                 CosObjNode *value,
                 CosError * COS_Nullable error);

COS_API extern const CosDictKeyCallbacks cos_dict_obj_node_key_callbacks;
COS_API extern const CosDictValueCallbacks cos_dict_obj_node_value_callbacks;

// MARK: - Iterator

/**
 * An iterator over the key-value pairs in a dictionary object.
 *
 * Initialize with @c cos_dict_obj_node_iterator_init and advance with
 * @c cos_dict_obj_node_iterator_next.  The iterator does not own any resources
 * and becomes invalid if the dictionary is mutated.
 */
typedef struct CosDictObjNodeIterator {
    /**
     * The underlying dictionary iterator.
     * @private
     */
    CosDictIterator base;
} CosDictObjNodeIterator;

/**
 * Initializes a dictionary object iterator.
 *
 * @param dict_obj The dictionary object to iterate over.
 *
 * @return An iterator positioned before the first entry.
 */
COS_API CosDictObjNodeIterator
cos_dict_obj_node_iterator_init(CosDictObjNode *dict_obj);

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
COS_API bool
cos_dict_obj_node_iterator_next(CosDictObjNodeIterator *iterator,
                           CosNameObjNode * COS_Nullable * COS_Nonnull out_key,
                           CosObjNode * COS_Nullable * COS_Nonnull out_value)
    COS_ATTR_ACCESS_WRITE_ONLY(2)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_DICT_OBJ_H */
