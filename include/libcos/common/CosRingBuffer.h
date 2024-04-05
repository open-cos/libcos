/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_RING_BUFFER_H
#define LIBCOS_COMMON_COS_RING_BUFFER_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/** @{ **/
/** @name Allocation/Deallocation */

/**
 * @brief Destroys a ring buffer.
 *
 * @param ring_buffer The ring buffer.
 */
void
cos_ring_buffer_destroy(CosRingBuffer *ring_buffer)
    COS_DEALLOCATOR_FUNC;

/**
 * @brief Creates a ring buffer.
 *
 * @param element_size The size of each element in the ring buffer.
 * @param capacity_hint A hint for the initial capacity of the ring buffer.
 *
 * @return The ring buffer, or @c NULL if memory allocation failed.
 */
CosRingBuffer * COS_Nullable
cos_ring_buffer_create(size_t element_size,
                       size_t capacity_hint)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_ring_buffer_destroy);

/** @} **/
/** @{ **/
/** @name Accessors */

/**
 * @brief Gets the number of items in a ring buffer.
 *
 * @param ring_buffer The ring buffer.
 *
 * @return The number of items in the ring buffer.
 */
size_t
cos_ring_buffer_get_count(const CosRingBuffer *ring_buffer);

/**
 * @brief Gets the capacity of a ring buffer.
 *
 * @param ring_buffer The ring buffer.
 *
 * @return The capacity of the ring buffer.
 */
size_t
cos_ring_buffer_get_capacity(const CosRingBuffer *ring_buffer);

/**
 * @brief Gets the item at a given index in a ring buffer.
 *
 * @param ring_buffer The ring buffer.
 * @param index The index of the item to get.
 * @param out_error On input, a pointer to an error object, or @c NULL.
 *
 * @return The item at the given index, or @c NULL if an error occurred.
 */
void * COS_Nullable
cos_ring_buffer_get_item(const CosRingBuffer *ring_buffer,
                         size_t index,
                         CosError * COS_Nullable out_error);

/**
 * @brief Gets the first item in a ring buffer.
 *
 * @param ring_buffer The ring buffer.
 *
 * @return The first item in the ring buffer, or @c NULL if the ring buffer is empty.
 */
void * COS_Nullable
cos_ring_buffer_get_first_item(const CosRingBuffer *ring_buffer);

/**
 * @brief Gets the last item in a ring buffer.
 *
 * @param ring_buffer The ring buffer.
 *
 * @return The last item in the ring buffer, or @c NULL if the ring buffer is empty.
 */
void * COS_Nullable
cos_ring_buffer_get_last_item(const CosRingBuffer *ring_buffer);

/** @} **/

/** @{ **/
/** @name Modification */

/**
 * @brief Pushes an item to the front of a ring buffer.
 *
 * @param ring_buffer The ring buffer.
 * @param item The item to push.
 * @param out_error On input, a pointer to an error object, or @c NULL.
 *
 * @return @c true if the item was pushed successfully, @c false if an error occurred.
 */
bool
cos_ring_buffer_push_front(CosRingBuffer *ring_buffer,
                           const void *item,
                           CosError * COS_Nullable out_error)
    COS_OWNERSHIP_HOLDS(2);

/**
 * @brief Pushes an item to the back of a ring buffer.
 *
 * @param ring_buffer The ring buffer.
 * @param item The item to push.
 * @param out_error On input, a pointer to an error object, or @c NULL.
 *
 * @return @c true if the item was pushed successfully, @c false if an error occurred.
 */
bool
cos_ring_buffer_push_back(CosRingBuffer *ring_buffer,
                          const void *item,
                          CosError * COS_Nullable out_error)
    COS_OWNERSHIP_HOLDS(2);

/**
 * @brief Pops an item from the front of a ring buffer.
 *
 * @param ring_buffer The ring buffer.
 * @param out_error On input, a pointer to an error object, or @c NULL.
 *
 * @return The item that was popped, or @c NULL if an error occurred.
 */
void * COS_Nullable
cos_ring_buffer_pop_front(CosRingBuffer *ring_buffer,
                          CosError * COS_Nullable out_error)
    COS_OWNERSHIP_RETURNS;

/**
 * @brief Pops an item from the back of a ring buffer.
 *
 * @param ring_buffer The ring buffer.
 * @param out_error On input, a pointer to an error object, or @c NULL.
 *
 * @return The item that was popped, or @c NULL if an error occurred.
 */
void * COS_Nullable
cos_ring_buffer_pop_back(CosRingBuffer *ring_buffer,
                         CosError * COS_Nullable out_error)
    COS_OWNERSHIP_RETURNS;

/** @} **/

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_RING_BUFFER_H */
