/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_LIST_H
#define LIBCOS_COMMON_COS_LIST_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief Allocates a new empty list.
 *
 * @return The new list, or @c NULL if memory allocation failed.
 */
CosList * COS_Nullable
cos_list_alloc(void)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

/**
 * @brief Frees a list.
 *
 * @param list The list to be freed.
 */
void
cos_list_free(CosList *list);

#pragma mark - Accessors

/**
 * @brief Gets the number of items in a list.
 *
 * @param list The list.
 *
 * @return The number of items in the list.
 */
size_t
cos_list_get_count(const CosList *list);

/**
 * @brief Gets the first item in a list.
 *
 * @param list The list.
 *
 * @return The first item in the list, or @c NULL if the list is empty.
 */
void * COS_Nullable
cos_list_get_first(const CosList *list);

/**
 * @brief Gets the last item in a list.
 *
 * @param list The list.
 *
 * @return The last item in the list, or @c NULL if the list is empty.
 */
void * COS_Nullable
cos_list_get_last(const CosList *list);

/**
 * @brief Gets the item at the given index in a list.
 *
 * @param list The list.
 * @param index The index of the item to get.
 * @param error On input, a pointer to an error object, or @c NULL.
 * On output, if an error occurred, the error object will be set with the error information.
 *
 * @return The item at the given index in the list, or @c NULL if the index is out of bounds.
 */
void * COS_Nullable
cos_list_get_at(const CosList *list,
                size_t index,
                CosError * COS_Nullable error);

#pragma mark - Operations

/**
 * @brief Prepends an item to a list.
 *
 * @param list The list.
 * @param item The item to prepend to the list.
 * @param error On input, a pointer to an error object, or @c NULL.
 * On output, if an error occurred, the error object will be set with the error information.
 *
 * @return @c true if the item was added to the list, @c false otherwise.
 */
bool
cos_list_prepend(CosList *list,
                 void *item,
                 CosError * COS_Nullable error);

/**
 * @brief Appends an item to a list.
 *
 * @param list The list.
 * @param item The item to append to the list.
 * @param error On input, a pointer to an error object, or @c NULL.
 * On output, if an error occurred, the error object will be set with the error information.
 *
 * @return @c true if the item was added to the list, @c false otherwise.
 */
bool
cos_list_append(CosList *list,
                void *item,
                CosError * COS_Nullable error);

/**
 * @brief Inserts an item at the given index in a list.
 *
 * @param list The list.
 * @param index The index at which to insert the item.
 * @param item The item to insert into the list.
 * @param error On input, a pointer to an error object, or @c NULL.
 * On output, if an error occurred, the error object will be set with the error information.
 *
 * @return @c true if the item was inserted into the list, @c false otherwise.
 */
bool
cos_list_insert(CosList *list,
                size_t index,
                void *item,
                CosError * COS_Nullable error);

/**
 * @brief Removes the item at the given index from a list.
 *
 * @param list The list.
 * @param index The index of the item to remove from the list.
 * @param error On input, a pointer to an error object, or @c NULL.
 * On output, if an error occurred, the error object will be set with the error information.
 *
 * @return @c true if the item was removed from the list, @c false otherwise.
 */
bool
cos_list_remove_at(CosList *list,
                   size_t index,
                   CosError * COS_Nullable error);

/**
 * @brief Removes the first item from a list.
 *
 * @param list The list.
 * @param error On input, a pointer to an error object, or @c NULL.
 * On output, if an error occurred, the error object will be set with the error information.
 *
 * @return @c true if the first item was removed from the list, @c false otherwise.
 */
bool
cos_list_remove_first(CosList *list,
                      CosError * COS_Nullable error);

/**
 * @brief Removes the last item from a list.
 *
 * @param list The list.
 * @param error On input, a pointer to an error object, or @c NULL.
 * On output, if an error occurred, the error object will be set with the error information.
 *
 * @return @c true if the last item was removed from the list, @c false otherwise.
 */
bool
cos_list_remove_last(CosList *list,
                     CosError * COS_Nullable error);

/**
 * @brief Removes all items from a list.
 *
 * @param list The list.
 */
void
cos_list_clear(CosList *list);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_LIST_H */
