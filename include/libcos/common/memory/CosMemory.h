/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_MEMORY_COS_MEMORY_H
#define LIBCOS_COMMON_MEMORY_COS_MEMORY_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @file CosMemory.h
 *
 * The library's memory allocation choke point. Every allocation in libcos goes
 * through @c cos_malloc / @c cos_calloc / @c cos_realloc / @c cos_free , which
 * dispatch to a single global @c CosMemoryAllocator .
 *
 * The default allocator wraps the C library. It can be replaced -- to plug in a
 * custom @c malloc , a fixed-size static arena where dynamic allocation is not
 * available, or an instrumented allocator that injects failures for testing --
 * with @c cos_set_memory_allocator .
 */

/**
 * A set of memory-management functions plus opaque user data passed to each.
 *
 * The three function pointers are required; @c user_data is forwarded to them
 * unchanged and may be @c NULL .
 */
struct CosMemoryAllocator {
    void * COS_Nullable (*malloc)(size_t size,
                                  void * COS_Nullable user_data);

    void * COS_Nullable (*realloc)(void * COS_Nullable ptr,
                                   size_t size,
                                   void * COS_Nullable user_data);

    void (*free)(void * COS_Nullable ptr,
                 void * COS_Nullable user_data);

    void * COS_Nullable user_data;
};

/**
 * Installs the global memory allocator.
 *
 * Must be called before any allocation is made, and is not safe to call
 * concurrently with allocation on another thread -- the intended use is a single
 * install at start-up.
 *
 * @param allocator The allocator to install, or @c NULL to reset to the default
 * (C library) allocator. A non-NULL allocator must provide all three function
 * pointers; the struct is copied.
 */
COS_API void
cos_set_memory_allocator(const CosMemoryAllocator * COS_Nullable allocator);

/**
 * Returns the currently installed memory allocator.
 *
 * @return A copy of the installed allocator.
 */
COS_API CosMemoryAllocator
cos_get_memory_allocator(void);

COS_API void
cos_free(void * COS_Nullable ptr)
    COS_DEALLOCATOR_FUNC_INDEX(1);

COS_API void * COS_Nullable
cos_malloc(size_t size)
    COS_ALLOCATOR_FUNC_SIZE(1)
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC_INDEX(cos_free, 1);

COS_API void * COS_Nullable
cos_calloc(size_t count,
           size_t size)
    COS_ALLOCATOR_FUNC_SIZES(1, 2)
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC_INDEX(cos_free, 1);

COS_API void * COS_Nullable
cos_realloc(void * COS_Nullable ptr,
            size_t size)
    COS_DEALLOCATOR_FUNC_INDEX(1)
    COS_ALLOCATOR_FUNC_SIZE(2)
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC_INDEX(cos_free, 1);

// MARK: - Out-of-memory handler

/**
 * Called when an allocation through the choke point has failed, to give the
 * application a chance to release memory and have the allocation retried.
 *
 * This is the C library's @c set_new_handler adapted to libcos's allocator: the
 * handler runs before @c NULL is returned to the caller. It should free whatever
 * memory it can reclaim and return @c true to have the allocation retried, or
 * @c false to give up (the caller then sees @c NULL ).
 *
 * The handler must not itself allocate through @c cos_malloc / @c cos_calloc /
 * @c cos_realloc : while it runs, a further allocation failure does not re-enter
 * it and simply returns @c NULL .
 *
 * @param size The byte size of the allocation that failed.
 * @param attempt How many times this allocation has failed so far: 1 on the
 * first failure, incrementing on each retry, so the handler can give up once it
 * has nothing left to free.
 * @param user_data The user data supplied to @c cos_set_out_of_memory_handler .
 *
 * @return @c true to retry the allocation, @c false to give up.
 */
typedef bool (*CosOutOfMemoryHandlerFunc)(size_t size,
                                          unsigned int attempt,
                                          void * COS_Nullable user_data);

/**
 * Installs the global out-of-memory handler, or removes it.
 *
 * Like the allocator, this is intended to be installed once before any
 * allocation is made, and is not safe to change concurrently with allocation on
 * another thread.
 *
 * @param handler The handler to install, or @c NULL to remove any handler. With
 * no handler installed a failed allocation returns @c NULL immediately, which is
 * the default.
 * @param user_data Opaque data forwarded to @p handler unchanged; may be
 * @c NULL .
 */
COS_API void
cos_set_out_of_memory_handler(CosOutOfMemoryHandlerFunc COS_Nullable handler,
                              void * COS_Nullable user_data);

// MARK: - Benign-malloc regions

/**
 * Marks the start of a benign-malloc region.
 *
 * Allocations made while a region is active are ones whose failure the caller is
 * prepared to absorb gracefully -- typically a speculative peek that treats a
 * failed read as "nothing there" and retries. Regions nest: each
 * @c cos_begin_benign_malloc must be balanced by a @c cos_end_benign_malloc .
 *
 * The default allocator is indifferent to this state; it exists so an
 * instrumented allocator (the OOM fault injector) can tell an absorbed failure
 * apart from one that must propagate. Outside a testing build this is a no-op.
 */
COS_API void
cos_begin_benign_malloc(void);

/**
 * Marks the end of a benign-malloc region opened by @c cos_begin_benign_malloc .
 */
COS_API void
cos_end_benign_malloc(void);

/**
 * Whether execution is currently inside a benign-malloc region.
 *
 * Intended for an instrumented allocator deciding whether a failure it injects
 * needs to propagate. Always @c false outside a testing build.
 *
 * @return @c true if at least one region is open.
 */
COS_API bool
cos_memory_in_benign_region(void);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_MEMORY_COS_MEMORY_H */
