/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/common/CosStack.h"

#include <libcos/common/CosArray.h>

#include <stdlib.h>

#include "Assert.h"

COS_ASSUME_NONNULL_BEGIN

struct CosStack {
    /**
     * The array that holds the stack elements.
     */
    CosArray *array;
};

CosStack *
cos_stack_create(size_t capacity_hint)
{
    CosStack *stack = NULL;
    CosArray *array = NULL;

    stack = calloc(1, sizeof(CosStack));
    if (!stack) {
        goto failure;
    }

    array = cos_array_create(sizeof(void *),
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
cos_stack_destroy(CosStack *stack)
{
    if (!stack) {
        return;
    }

    cos_array_destroy(stack->array);
    free(stack);
}

size_t
cos_stack_get_count(const CosStack *stack)
{
    COS_PARAMETER_ASSERT(stack != NULL);
    if (!stack) {
        return 0;
    }

    return cos_array_get_count(stack->array);
}

bool
cos_stack_push(CosStack *stack,
               const void *element,
               CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(stack != NULL);
    COS_PARAMETER_ASSERT(element != NULL);
    if (!stack || !element) {
        return false;
    }

    return cos_array_push_last_item(stack->array,
                                    &element,
                                    out_error);
}

void *
cos_stack_pop(CosStack *stack,
              CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(stack != NULL);
    if (!stack) {
        return NULL;
    }

    void *element = NULL;
    if (!cos_array_pop_last_item(stack->array,
                                 &element,
                                 out_error)) {
        return NULL;
    }
    return element;
}

COS_ASSUME_NONNULL_END
