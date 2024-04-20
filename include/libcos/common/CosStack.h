/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_STACK_H
#define LIBCOS_COMMON_COS_STACK_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

void
cos_stack_destroy(CosStack *stack)
    COS_DEALLOCATOR_FUNC;

CosStack * COS_Nullable
cos_stack_create(size_t capacity_hint)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_stack_destroy);

size_t
cos_stack_get_count(const CosStack *stack);

bool
cos_stack_push(CosStack *stack,
               const void *element,
               CosError * COS_Nullable out_error)
    COS_OWNERSHIP_HOLDS(2)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

void * COS_Nullable
cos_stack_pop(CosStack *stack,
              CosError * COS_Nullable out_error)
    COS_OWNERSHIP_RETURNS
    COS_ATTR_ACCESS_WRITE_ONLY(2);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_STACK_H */
