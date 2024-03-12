/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_MEMORY_COS_MEMORY_H
#define LIBCOS_COMMON_MEMORY_COS_MEMORY_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

void
cos_free(CosAllocator * COS_Nullable allocator,
         void * COS_Nullable ptr)
    COS_DEALLOCATOR_FUNC_INDEX(2);

void * COS_Nullable
cos_alloc(CosAllocator * COS_Nullable allocator,
          size_t size)
    COS_ALLOCATOR_FUNC_SIZE(2)
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC_INDEX(cos_free, 2);

void * COS_Nullable
cos_realloc(CosAllocator * COS_Nullable allocator,
            void * COS_Nullable ptr,
            size_t size)
    COS_DEALLOCATOR_FUNC_INDEX(2)
    COS_ALLOCATOR_FUNC_SIZE(3)
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC_INDEX(cos_free, 2);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_MEMORY_COS_MEMORY_H */
