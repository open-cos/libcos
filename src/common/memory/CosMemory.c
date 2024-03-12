/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/common/memory/CosMemory.h"

#include "common/Assert.h"

#include <libcos/common/memory/CosAllocator.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

void *
cos_alloc(CosAllocator * COS_Nullable allocator,
          size_t size)
{
    COS_PARAMETER_ASSERT(size > 0);

    if (allocator) {
        return cos_allocator_alloc(COS_nonnull_cast(allocator),
                                   size);
    }
    else {
        return malloc(size);
    }
}

void *
cos_realloc(CosAllocator * COS_Nullable allocator,
            void * COS_Nullable ptr,
            size_t size)
{
    COS_PARAMETER_ASSERT(ptr || size > 0);
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
