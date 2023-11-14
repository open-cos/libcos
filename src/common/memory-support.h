//
// Created by david on 24/08/23.
//

#ifndef LIBCOS_COMMON_MEMORY_SUPPORT_H
#define LIBCOS_COMMON_MEMORY_SUPPORT_H

#include <libcos/common/CosDefines.h>

#include <stddef.h>

void *
cos_grow(void *ptr,
         size_t size,
         size_t element_size,
         size_t *new_size);

#endif /* LIBCOS_COMMON_MEMORY_SUPPORT_H */
