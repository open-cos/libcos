//
// Created by david on 16/08/23.
//

#ifndef LIBCOS_COS_STRING_IMPL_H
#define LIBCOS_COS_STRING_IMPL_H

#include "common/CosString.h"

#include <stddef.h>

/**
 * A nul-terminated character array.
 */
struct CosString {
    /**
     * The nul-terminated character array.
     */
    char *str;

    /**
     * The number of characters in the string, including the nul-terminator.
     */
    size_t size;
};

#endif /* LIBCOS_COS_STRING_IMPL_H */
