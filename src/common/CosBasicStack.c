/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/common/CosBasicStack.h"

#include <libcos/common/CosArray.h>

#include <stdlib.h>

#include "Assert.h"

COS_ASSUME_NONNULL_BEGIN

struct CosBasicStack {
    CosArray *array;
};

CosBasicStack *
cos_basic_stack_create(size_t element_size,
                       size_t capacity_hint)
{
    COS_PARAMETER_ASSERT(element_size > 0);
    if (element_size == 0) {
        return NULL;
    }

    CosBasicStack *stack = NULL;
    CosArray *array = NULL;

    stack = calloc(1, sizeof(CosBasicStack));
    if (!stack) {
        goto failure;
    }

    array = cos_array_create(element_size,
                             NULL,
                             capacity_hint);
    if (!array) {
        goto failure;
    }

    stack->array = array;

    return stack;

failure:
    if (stack) {
        free(stack);
    }
    if (array) {
        cos_array_destroy(array);
    }
    return NULL;
}

void
cos_basic_stack_destroy(CosBasicStack *stack)
{
    if (!stack) {
        return;
    }

    cos_array_destroy(stack->array);
    free(stack);
}

size_t
cos_basic_stack_get_count(const CosBasicStack *stack)
{
    COS_PARAMETER_ASSERT(stack != NULL);
    if (!stack) {
        return 0;
    }

    return cos_array_get_count(stack->array);
}

bool
cos_basic_stack_push(CosBasicStack *stack,
                     const void *element,
                     CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(stack != NULL);
    COS_PARAMETER_ASSERT(element != NULL);
    if (!stack || !element) {
        return false;
    }

    return cos_array_append_item(stack->array,
                                 element,
                                 out_error);
}

bool
cos_basic_stack_pop(CosBasicStack *stack,
                    void *out_element,
                    CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(stack != NULL);
    COS_PARAMETER_ASSERT(out_element != NULL);
    if (!stack || !out_element) {
        return false;
    }

    return cos_array_pop_last_item(stack->array,
                                   out_element,
                                   out_error);
}

COS_ASSUME_NONNULL_END
