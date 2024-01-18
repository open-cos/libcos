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

CosArray * COS_Nullable
cos_array_alloc(size_t element_size,
                CosArrayCallbacks callbacks,
                size_t capacity_hint)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_array_free(CosArray *array);

#pragma mark - Accessors

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
 * @param error On input, a pointer to an error object, or @c NULL.
 *
 * @return The item at the given index, or @c NULL if an error occurred.
 */
void * COS_Nullable
cos_array_get_item(const CosArray *array,
                   size_t index,
                   CosError * COS_Nullable error);

#pragma mark Insertion

bool
cos_array_insert_item(CosArray *array,
                      size_t index,
                      void *item,
                      CosError * COS_Nullable error);

bool
cos_array_append_item(CosArray *array,
                      void *item,
                      CosError * COS_Nullable error);

bool
cos_array_insert_items(CosArray *array,
                       size_t index,
                       void *items,
                       size_t count,
                       CosError * COS_Nullable error);

bool
cos_array_append_items(CosArray *array,
                       void *items,
                       size_t count,
                       CosError * COS_Nullable error);

#pragma mark Removal

bool
cos_array_remove_item(CosArray *array,
                      size_t index,
                      CosError * COS_Nullable error);

bool
cos_array_remove_items(CosArray *array,
                       size_t index,
                       size_t count,
                       CosError * COS_Nullable error);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_ARRAY_H */
