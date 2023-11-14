//
// Created by david on 24/08/23.
//

#include "memory-support.h"

#include <stdlib.h>

void *
cos_grow(void *ptr,
         size_t size,
         size_t element_size,
         size_t *out_size)
{
    if (!ptr) {
        return NULL;
    }

    const size_t new_size = size * 2;

    void * const new_ptr = realloc(ptr, new_size * element_size);
    if (!new_ptr) {
        return NULL;
    }

    if (out_size) {
        *out_size = new_size;
    }

    return new_ptr;
}
