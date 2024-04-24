/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_STACK_INL
#define LIBCOS_COMMON_COS_STACK_INL

#include <libcos/common/CosBasicStack.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosStack.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

COS_STATIC_INLINE
CosStack *
cos_stack_create(size_t capacity_hint)
{
    return cos_basic_stack_create(sizeof(void *),
                                  capacity_hint);
}

COS_STATIC_INLINE
void
cos_stack_destroy(CosStack *stack)
{
    cos_basic_stack_destroy(stack);
}

COS_STATIC_INLINE
size_t
cos_stack_get_count(const CosStack *stack)
{
    return cos_basic_stack_get_count(stack);
}

COS_STATIC_INLINE
bool
cos_stack_push(CosStack *stack,
               const void *element,
               CosError * COS_Nullable out_error)
{
    return cos_basic_stack_push(stack,
                                &element,
                                out_error);
}

COS_STATIC_INLINE
void *
cos_stack_pop(CosStack *stack,
              CosError * COS_Nullable out_error)
{
    void *element = NULL;
    if (!cos_basic_stack_pop(stack,
                             &element,
                             out_error)) {
        return NULL;
    }
    return element;
}

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_STACK_INL */
