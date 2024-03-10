/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/common/memory/CosAllocator.h"

#include "common/Assert.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosAllocator {
    CosAllocatorCallbacks callbacks;

    void * COS_Nullable user_data;

    CosAllocator * COS_Nullable parent;
};

// MARK: Creation and Destruction

CosAllocator *
cos_allocator_create(CosAllocator * COS_Nullable allocator,
                     const CosAllocatorCallbacks *callbacks,
                     void * COS_Nullable user_data)
{
    COS_PARAMETER_ASSERT(callbacks != NULL);
    if (!callbacks) {
        return NULL;
    }

    CosAllocator * const parent_allocator = allocator ? allocator : CosAllocatorDefault;
    COS_ASSERT(parent_allocator != NULL, "Expected a parent allocator");

    CosAllocator *new_allocator = cos_allocator_alloc(COS_nonnull_cast(parent_allocator),
                                                      sizeof(CosAllocator));
    if (!new_allocator) {
        return NULL;
    }

    new_allocator->callbacks = *callbacks;

    // Retain the user data if necessary.
    if (callbacks->retain && user_data) {
        new_allocator->user_data = callbacks->retain(COS_nonnull_cast(user_data));
    }
    else {
        new_allocator->user_data = user_data;
    }

    new_allocator->parent = parent_allocator;

    return new_allocator;
}

void
cos_allocator_destroy(CosAllocator *allocator)
{
    if (!allocator) {
        return;
    }

    // Release the user data if necessary.
    void * const user_data = allocator->user_data;
    if (allocator->callbacks.release && user_data) {
        allocator->callbacks.release(user_data);
    }

    if (allocator->parent) {
        cos_allocator_dealloc(COS_nonnull_cast(allocator->parent),
                              allocator);
    }
    else {
        free(allocator);
    }
}

// MARK: Member Functions

void *
cos_allocator_alloc(CosAllocator *allocator,
                    size_t size)
{
    COS_PARAMETER_ASSERT(allocator != NULL);
    if (!allocator || !allocator->callbacks.alloc) {
        return NULL;
    }

    return allocator->callbacks.alloc(size,
                                      allocator->user_data);
}

void *
cos_allocator_realloc(CosAllocator *allocator,
                      void * COS_Nullable ptr,
                      size_t size)
{
    COS_PARAMETER_ASSERT(allocator != NULL);
    if (!allocator || !allocator->callbacks.realloc) {
        return NULL;
    }

    return allocator->callbacks.realloc(ptr,
                                        size,
                                        allocator->user_data);
}

void
cos_allocator_dealloc(CosAllocator *allocator,
                      void *ptr)
{
    COS_PARAMETER_ASSERT(allocator != NULL);
    if (!allocator || !allocator->callbacks.dealloc || !ptr) {
        return;
    }

    allocator->callbacks.dealloc(ptr,
                                 allocator->user_data);
}

// MARK: - Default Allocator

static void * COS_Nullable
cos_allocator_default_alloc_(size_t size,
                             COS_ATTR_UNUSED void * COS_Nullable user_data)
{
    return malloc(size);
}

static void * COS_Nullable
cos_allocator_default_realloc_(void * COS_Nullable ptr,
                               size_t size,
                               COS_ATTR_UNUSED void * COS_Nullable user_data)
{
    return realloc(ptr, size);
}

static void
cos_allocator_default_dealloc_(void *ptr,
                               COS_ATTR_UNUSED void * COS_Nullable user_data)
{
    free(ptr);
}

static CosAllocator cos_allocator_default_ = {
    .callbacks = {
        .alloc = &cos_allocator_default_alloc_,
        .realloc = &cos_allocator_default_realloc_,
        .dealloc = &cos_allocator_default_dealloc_,
        .retain = NULL,
        .release = NULL,
    },
    .user_data = NULL,
    .parent = NULL,
};

CosAllocator * const CosAllocatorDefault = &cos_allocator_default_;

COS_ASSUME_NONNULL_END
