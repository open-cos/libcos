/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_BASIC_STACK_H
#define LIBCOS_COMMON_COS_BASIC_STACK_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

void
cos_basic_stack_destroy(CosBasicStack *stack)
    COS_DEALLOCATOR_FUNC;

CosBasicStack * COS_Nullable
cos_basic_stack_create(size_t element_size,
                       size_t capacity_hint)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_basic_stack_destroy);

size_t
cos_basic_stack_get_count(const CosBasicStack *stack);

bool
cos_basic_stack_push(CosBasicStack *stack,
                     const void *element,
                     CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_READ_ONLY(2)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

bool
cos_basic_stack_pop(CosBasicStack *stack,
                    void *out_element,
                    CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(2)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_BASIC_STACK_H */
