/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/common/memory/CosMemory.h"

#include "common/Assert.h"
#include "common/CosCheckedArith.h"

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

// MARK: - Default (C library) allocator

static void * COS_Nullable
cos_default_malloc_(size_t size,
                    COS_ATTR_UNUSED void * COS_Nullable user_data)
{
    return malloc(size);
}

static void * COS_Nullable
cos_default_realloc_(void * COS_Nullable ptr,
                     size_t size,
                     COS_ATTR_UNUSED void * COS_Nullable user_data)
{
    return realloc(ptr, size);
}

static void
cos_default_free_(void *ptr,
                  COS_ATTR_UNUSED void * COS_Nullable user_data)
{
    free(ptr);
}

#define COS_DEFAULT_ALLOCATOR_INITIALIZER \
    {                                     \
        .malloc = &cos_default_malloc_,   \
        .realloc = &cos_default_realloc_, \
        .free = &cos_default_free_,       \
        .user_data = NULL,                \
    }

// The currently installed allocator. Not guarded: the documented contract is to
// install once before any allocation, so there is no writer racing the readers.
static CosMemoryAllocator cos_allocator_ = COS_DEFAULT_ALLOCATOR_INITIALIZER;

// The installed out-of-memory handler, if any, and its user data. Same
// install-once contract as the allocator above. cos_in_out_of_memory_handler_
// guards against re-entry: an allocation that fails while the handler runs must
// not invoke the handler again, or a handler that allocates could recurse
// without end.
static CosOutOfMemoryHandlerFunc COS_Nullable cos_out_of_memory_handler_ = NULL;
static void * COS_Nullable cos_out_of_memory_handler_user_data_ = NULL;
static bool cos_in_out_of_memory_handler_ = false;

// MARK: - Installation

void
cos_set_memory_allocator(const CosMemoryAllocator * COS_Nullable allocator)
{
    if (!allocator) {
        const CosMemoryAllocator default_allocator = COS_DEFAULT_ALLOCATOR_INITIALIZER;
        cos_allocator_ = default_allocator;
        return;
    }

    COS_API_PARAM_CHECK(allocator->malloc != NULL);
    COS_API_PARAM_CHECK(allocator->realloc != NULL);
    COS_API_PARAM_CHECK(allocator->free != NULL);
    if (COS_UNLIKELY(!allocator->malloc || !allocator->realloc || !allocator->free)) {
        return;
    }

    cos_allocator_ = *allocator;
}

CosMemoryAllocator
cos_get_memory_allocator(void)
{
    return cos_allocator_;
}

// MARK: - Out-of-memory handler

void
cos_set_out_of_memory_handler(CosOutOfMemoryHandlerFunc COS_Nullable handler,
                              void * COS_Nullable user_data)
{
    cos_out_of_memory_handler_ = handler;
    cos_out_of_memory_handler_user_data_ = user_data;
}

// Invokes the out-of-memory handler after a failed allocation of the given size,
// returning whether the caller should retry. Returns false when no handler is
// installed or when already inside the handler (see cos_in_out_of_memory_handler_).
// Recovery is attempted regardless of any benign-malloc region: the benign flag
// governs how a failure is classified for the test injector, not whether an
// application-supplied handler gets a chance to free memory and retry.
static bool
cos_out_of_memory_should_retry_(size_t size,
                                unsigned int attempt)
{
    if (!cos_out_of_memory_handler_ || cos_in_out_of_memory_handler_) {
        return false;
    }

    cos_in_out_of_memory_handler_ = true;
    const bool retry = cos_out_of_memory_handler_(size,
                                                  attempt,
                                                  cos_out_of_memory_handler_user_data_);
    cos_in_out_of_memory_handler_ = false;
    return retry;
}

// MARK: - Allocation

void *
cos_malloc(size_t size)
{
    COS_API_PARAM_CHECK(size > 0);

    void *ptr = cos_allocator_.malloc(size, cos_allocator_.user_data);
    for (unsigned int attempt = 1;
         !ptr && cos_out_of_memory_should_retry_(size, attempt);
         attempt++) {
        ptr = cos_allocator_.malloc(size, cos_allocator_.user_data);
    }
    return ptr;
}

void *
cos_calloc(size_t count,
           size_t size)
{
    COS_API_PARAM_CHECK(count > 0);
    COS_API_PARAM_CHECK(size > 0);

    // There is no calloc callback, so the zero-initialised allocation is
    // synthesised from a single cos_malloc plus memset -- one choke-point call,
    // so one fault-injection point and one accounted block. The total is
    // computed up front with an overflow check to keep calloc's count*size
    // contract.
    size_t total_size;
    if (COS_UNLIKELY(cos_ckd_mul_size_(&total_size, count, size))) {
        return NULL;
    }

    void * const ptr = cos_malloc(total_size);
    if (!ptr) {
        return NULL;
    }

    memset(ptr, 0, total_size);
    return ptr;
}

void *
cos_realloc(void * COS_Nullable ptr,
            size_t size)
{
    COS_API_PARAM_CHECK(ptr || size > 0);
    if (COS_UNLIKELY(!ptr && size == 0)) {
        return NULL;
    }

    void *new_ptr = cos_allocator_.realloc(ptr, size, cos_allocator_.user_data);
    for (unsigned int attempt = 1;
         !new_ptr && cos_out_of_memory_should_retry_(size, attempt);
         attempt++) {
        new_ptr = cos_allocator_.realloc(ptr, size, cos_allocator_.user_data);
    }
    return new_ptr;
}

void
cos_free(void * COS_Nullable ptr)
{
    if (!ptr) {
        return;
    }

    cos_allocator_.free(ptr, cos_allocator_.user_data);
}

// MARK: - Benign-malloc regions

#ifdef COS_OOM_TESTING

// The nesting depth of the currently open benign-malloc regions. Like the
// allocator above, this is not guarded: the fault injector that reads it runs
// single-threaded around the operation under test. Only compiled in when OOM
// testing is enabled; production builds carry none of this.
static size_t cos_benign_depth_ = 0;

void
cos_begin_benign_malloc(void)
{
    cos_benign_depth_ += 1;
}

void
cos_end_benign_malloc(void)
{
    COS_ASSERT(cos_benign_depth_ > 0,
               "Unbalanced cos_end_benign_malloc without a matching begin");
    if (cos_benign_depth_ > 0) {
        cos_benign_depth_ -= 1;
    }
}

bool
cos_memory_in_benign_region(void)
{
    return cos_benign_depth_ > 0;
}

#else

void
cos_begin_benign_malloc(void)
{
    // No-op: benign-malloc tracking is a testing-only facility.
}

void
cos_end_benign_malloc(void)
{
    // No-op: benign-malloc tracking is a testing-only facility.
}

bool
cos_memory_in_benign_region(void)
{
    return false;
}

#endif /* COS_OOM_TESTING */

COS_ASSUME_NONNULL_END
