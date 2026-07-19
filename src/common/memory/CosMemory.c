/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/common/memory/CosMemory.h"

#include "common/Assert.h"
#include "common/CosCheckedArith.h"

#include <libcos/common/memory/CosAllocator.h>

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

void *
cos_alloc(CosAllocator * COS_Nullable allocator,
          size_t size)
{
    COS_API_PARAM_CHECK(size > 0);

    if (allocator) {
        return cos_allocator_alloc(COS_nonnull_cast(allocator),
                                   size);
    }
    else {
        return malloc(size);
    }
}

void *
cos_calloc(CosAllocator * COS_Nullable allocator,
           size_t count,
           size_t size)
{
    COS_API_PARAM_CHECK(count > 0);
    COS_API_PARAM_CHECK(size > 0);

    if (!allocator) {
        return calloc(count, size);
    }

    // A custom allocator has no calloc callback, so the zero-initialised
    // allocation is synthesised from a single alloc plus memset. Computing the
    // total up front with an overflow check keeps the count*size contract that
    // calloc guarantees, and routes through cos_alloc so the allocation is a
    // single choke-point call (one fault-injection point, one accounted block).
    size_t total_size;
    if (COS_UNLIKELY(cos_ckd_mul_size_(&total_size, count, size))) {
        return NULL;
    }

    void * const ptr = cos_alloc(allocator, total_size);
    if (!ptr) {
        return NULL;
    }

    memset(ptr, 0, total_size);
    return ptr;
}

void *
cos_realloc(CosAllocator * COS_Nullable allocator,
            void * COS_Nullable ptr,
            size_t size)
{
    COS_API_PARAM_CHECK(ptr || size > 0);
    if (COS_UNLIKELY(!ptr && size == 0)) {
        return NULL;
    }

    if (allocator) {
        return cos_allocator_realloc(COS_nonnull_cast(allocator),
                                     ptr,
                                     size);
    }
    else {
        return realloc(ptr, size);
    }
}

void
cos_free(CosAllocator * COS_Nullable allocator,
         void * COS_Nullable ptr)
{
    if (!ptr) {
        return;
    }

    if (allocator) {
        cos_allocator_dealloc(COS_nonnull_cast(allocator),
                              ptr);
    }
    else {
        free(ptr);
    }
}

COS_ASSUME_NONNULL_END
