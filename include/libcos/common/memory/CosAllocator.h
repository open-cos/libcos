/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_MEMORY_COS_ALLOCATOR_H
#define LIBCOS_COMMON_MEMORY_COS_ALLOCATOR_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef void * COS_Nullable (*CosAllocatorAllocCallback)(size_t size,
                                                         void * COS_Nullable user_data);

typedef void * COS_Nullable (*CosAllocatorReallocCallback)(void * COS_Nullable ptr,
                                                           size_t size,
                                                           void * COS_Nullable user_data);

typedef void (*CosAllocatorDeallocCallback)(void *ptr,
                                            void * COS_Nullable user_data);

typedef void * COS_Nullable (*CosAllocatorRetainCallback)(void *user_data);

typedef void (*CosAllocatorReleaseCallback)(void *user_data);

typedef struct CosAllocatorCallbacks {
    CosAllocatorAllocCallback alloc;

    CosAllocatorReallocCallback COS_Nullable realloc;

    CosAllocatorDeallocCallback COS_Nullable dealloc;

    CosAllocatorRetainCallback COS_Nullable retain;

    CosAllocatorReleaseCallback COS_Nullable release;

} CosAllocatorCallbacks;

/** @name Creation and Destruction */
/** @{ **/

/**
 * @brief Destroys an allocator.
 *
 * @param allocator The allocator.
 */
void
cos_allocator_destroy(CosAllocator *allocator)
    COS_DEALLOCATOR_FUNC;

/**
 * @brief Creates a new allocator.
 *
 * @param allocator The existing allocator to use for the allocation, or @c NULL to use the default
 * allocator.
 * @param callbacks The allocator callbacks.
 * @param user_data The user data to pass to the allocator callbacks, or @c NULL if no user data
 * is needed.
 *
 * @return The new allocator, or @c NULL if the allocation failed.
 */
CosAllocator * COS_Nullable
cos_allocator_create(CosAllocator * COS_Nullable allocator,
                     const CosAllocatorCallbacks *callbacks,
                     void * COS_Nullable user_data)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_allocator_destroy);

/** @} **/

/** @name Memory Management */
/** @{ **/

/**
 * @brief Deallocates the specified block of memory.
 *
 * The block of memory must have been allocated using the same allocator that is
 * used to deallocate it. If the block of memory was not allocated using the same
 * allocator, the behavior is undefined.
 *
 * @param allocator The allocator.
 * @param ptr The pointer to the block of memory to deallocate. If the pointer is @c NULL, this
 * function does nothing.
 */
void
cos_allocator_dealloc(CosAllocator *allocator,
                      void * COS_Nullable ptr)
    COS_DEALLOCATOR_FUNC_INDEX(2);

/**
 * @brief Allocates a block of memory of the specified size.
 *
 * @param allocator The allocator.
 * @param size The size of the block of memory to allocate.
 *
 * @return A pointer to the allocated block of memory, or @c NULL if the allocation failed.
 * The returned pointer must be deallocated using the same allocator that was used to allocate it.
 */
void * COS_Nullable
cos_allocator_alloc(CosAllocator *allocator,
                    size_t size)
    COS_ALLOCATOR_FUNC_SIZE(2)
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC_INDEX(cos_allocator_dealloc, 2);

/**
 * @brief Reallocates a block of memory to the specified size.
 *
 * @param allocator The allocator.
 * @param ptr The pointer to the block of memory to reallocate, or @c NULL to allocate a new block
 * of memory.
 * @param size The new size of the block of memory, or @c 0 to deallocate the block of memory
 * pointed to by @a ptr.
 *
 * @return A pointer to the reallocated block of memory, or @c NULL if the reallocation failed.
 * The returned pointer must be deallocated using the same allocator that was used to reallocate it.
 */
void * COS_Nullable
cos_allocator_realloc(CosAllocator *allocator,
                      void * COS_Nullable ptr,
                      size_t size)
    COS_DEALLOCATOR_FUNC_INDEX(2)
    COS_ALLOCATOR_FUNC_SIZE(3)
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC_INDEX(cos_allocator_dealloc, 2);

/** @} **/

/**
 * @brief The default allocator.
 *
 * This allocator uses the standard C library functions for memory allocation.
 */
extern CosAllocator * const CosAllocatorDefault;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_MEMORY_COS_ALLOCATOR_H */
