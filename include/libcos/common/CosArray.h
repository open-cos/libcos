/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COS_ARRAY_H
#define LIBCOS_COS_ARRAY_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef void (*CosArrayRetainItemCallback)(void *item);
typedef void (*CosArrayReleaseItemCallback)(void *item);
typedef bool (*CosArrayEqualItemsCallback)(const void *item1,
                                           const void *item2);

typedef struct CosArrayCallbacks {
    CosArrayRetainItemCallback COS_Nullable retain;
    CosArrayReleaseItemCallback COS_Nullable release;
    CosArrayEqualItemsCallback COS_Nullable equal;
} CosArrayCallbacks;

void
cos_array_destroy(CosArray *array)
    COS_DEALLOCATOR_FUNC;

CosArray * COS_Nullable
cos_array_create(size_t element_size,
                 const CosArrayCallbacks * COS_Nullable callbacks,
                 size_t capacity_hint)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_array_destroy);

// MARK: - Accessors

/**
 * @brief Gets the number of items in an array.
 *
 * @param array The array.
 *
 * @return The number of items in the array.
 */
size_t
cos_array_get_count(const CosArray *array);

/**
 * @brief Gets the capacity of an array.
 *
 * @param array The array.
 *
 * @return The capacity of the array.
 */
size_t
cos_array_get_capacity(const CosArray *array);

/**
 * @brief Gets the item at a given index in an array.
 *
 * @param array The array.
 * @param index The index of the item to get.
 * @param out_item The output item.
 * @param out_error On input, a pointer to an error object, or @c NULL.
 *
 * @return The item at the given index, or @c NULL if an error occurred.
 */
bool
cos_array_get_item(const CosArray *array,
                   size_t index,
                   void *out_item,
                   CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(3)
    COS_ATTR_ACCESS_WRITE_ONLY(4);

/** @name Insertion */
/** @{ **/

bool
cos_array_insert_item(CosArray *array,
                      size_t index,
                      const void *item,
                      CosError * COS_Nullable error);

bool
cos_array_append_item(CosArray *array,
                      const void *item,
                      CosError * COS_Nullable error);

bool
cos_array_insert_items(CosArray *array,
                       size_t index,
                       const void *items,
                       size_t count,
                       CosError * COS_Nullable error);

bool
cos_array_append_items(CosArray *array,
                       const void *items,
                       size_t count,
                       CosError * COS_Nullable error);

/** @} */

/** @name Removal */
/** @{ **/

bool
cos_array_remove_item(CosArray *array,
                      size_t index,
                      CosError * COS_Nullable error);

bool
cos_array_remove_last_item(CosArray *array,
                           CosError * COS_Nullable error);

bool
cos_array_remove_items(CosArray *array,
                       size_t index,
                       size_t count,
                       CosError * COS_Nullable error);

/** @} **/

/** @{ **/
/**
 * @name Push and Pop Convenience Functions
 */

bool
cos_array_push_last_item(CosArray *array,
                         const void *item,
                         CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_READ_ONLY(2)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

bool
cos_array_pop_last_item(CosArray *array,
                        void *out_item,
                        CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(2)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

/** @} **/

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_ARRAY_H */
