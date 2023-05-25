//
// Created by david on 20/05/23.
//

#ifndef LIBCOS_COS_VECTOR_H
#define LIBCOS_COS_VECTOR_H

#include <stdbool.h>
#include <stddef.h>

typedef struct CosVector CosVector;

CosVector *
cos_vector_alloc(size_t element_size);

void
cos_vector_free(CosVector *vector);

size_t
cos_vector_get_element_size(const CosVector *vector);

unsigned int
cos_vector_get_capacity(const CosVector *vector);

unsigned int
cos_vector_get_length(const CosVector *vector);

void * const *
cos_vector_get_elements(const CosVector *vector);

//

bool
cos_vector_add_element(CosVector *vector,
                       void *element);

bool
cos_vector_insert_element(CosVector *vector,
                          unsigned int index,
                          void *element);

bool
cos_vector_remove_element(CosVector *vector,
                          unsigned int index);

#endif // LIBCOS_COS_VECTOR_H
