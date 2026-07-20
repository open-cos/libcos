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

// MARK: - Allocation

void *
cos_malloc(size_t size)
{
    COS_API_PARAM_CHECK(size > 0);

    return cos_allocator_.malloc(size, cos_allocator_.user_data);
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

    return cos_allocator_.realloc(ptr, size, cos_allocator_.user_data);
}

void
cos_free(void * COS_Nullable ptr)
{
    if (!ptr) {
        return;
    }

    cos_allocator_.free(ptr, cos_allocator_.user_data);
}

COS_ASSUME_NONNULL_END
