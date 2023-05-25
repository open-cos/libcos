//
// Created by david on 20/05/23.
//

#include "common/CosVector.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define COS_ASSERT(condition) do { if ((condition) == false) { abort(); } } while(0)

struct CosVector {
    void **elements;

    size_t element_size;
    unsigned int capacity;
    unsigned int length;
};

static bool
cos_vector_needs_resize_(const CosVector *vector);

static bool
cos_vector_resize_(CosVector *vector);

CosVector *
cos_vector_alloc(size_t element_size)
{
    if (element_size == 0) {
        return NULL;
    }

    CosVector * const vector = malloc(sizeof(CosVector));
    if (!vector) {
        return NULL;
    }

    vector->element_size = element_size;

    vector->elements = NULL;
    vector->length = 0;
    vector->capacity = 0;

    return vector;
}

void
cos_vector_free(CosVector *vector)
{
    if (!vector) {
        return;
    }

    free(vector->elements);
    vector->elements = NULL;

    vector->capacity = 0;
    vector->length = 0;

    free(vector);
}

size_t
cos_vector_get_element_size(const CosVector *vector)
{
    return vector->element_size;
}

unsigned int
cos_vector_get_capacity(const CosVector *vector)
{
    return vector->capacity;
}

unsigned int
cos_vector_get_length(const CosVector *vector)
{
    return vector->length;
}

void * const *
cos_vector_get_elements(const CosVector *vector)
{
    return vector->elements;
}

bool
cos_vector_add_element(CosVector *vector,
                       void *element)
{
    if ((vector->length + 1) > vector->capacity) {
        if (!cos_vector_resize_(vector)) {
            return false;
        }
    }

    // We should have enough space for (at least) one element.
    COS_ASSERT(vector->capacity > vector->length);

    vector->elements[vector->length] = element;
    vector->length++;

    return true;
}

bool
cos_vector_insert_element(CosVector *vector,
                          unsigned int index,
                          void *element)
{
    if (index > vector->length) {
        return false;
    }

    if (cos_vector_needs_resize_(vector)) {
        if (!cos_vector_resize_(vector)) {
            return false;
        }
    }

    if (index < vector->length) {
        // How many elements need to be shifted.
        const unsigned int move_count = vector->length - index;
        if (move_count > 0) {
            memmove(vector->elements[index] + 1,
                    vector->elements[index],
                    move_count * vector->element_size);
        }
    }

    vector->elements[index] = element;
    vector->length++;

    return true;
}

bool
cos_vector_remove_element(CosVector *vector,
                          unsigned int index)
{
    if (index >= vector->length) {
        return false;
    }

    const unsigned int move_count = vector->length - index;
    if (move_count > 0) {
        memmove(vector->elements[index],
                vector->elements[index] + 1,
                move_count * vector->element_size);
    }

    vector->length--;

    return false;
}

static bool
cos_vector_needs_resize_(const CosVector *vector)
{
    if (vector->capacity == 0 || (vector->capacity - 1) < vector->length) {
        return true;
    }
    else {
        return false;
    }
}

static bool
cos_vector_resize_(CosVector *vector)
{
    const unsigned int original_capacity = vector->capacity;

    const unsigned int new_capacity = (original_capacity > 0) ? (original_capacity * 2) : 1;
    COS_ASSERT(new_capacity > 0);

    const size_t new_size = new_capacity * vector->element_size;

    void ** const elements = realloc(vector->elements, new_size);
    if (!elements) {
        return false;
    }

    vector->elements = elements;
    vector->capacity = new_capacity;

    return true;
}
