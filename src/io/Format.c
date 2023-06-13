//
// Created by david on 10/06/23.
//

#include "libcos/io/Format.h"

#include "common/Assert.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

char *
cos_asprintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char * const result = cos_vasprintf(fmt, args);
    va_end(args);

    return result;
}

char *
cos_vasprintf(const char *format,
              va_list args)
{
    // Make a copy of the variadic args because we need to consume them twice.
    va_list args_copy;
    va_copy(args_copy, args);
    // Note that the formatted length does not include the null terminator.
    const int formatted_length = vsnprintf(NULL,
                                           0,
                                           format,
                                           args_copy);
    va_end(args_copy);

    if (formatted_length < 0) {
        // An error occurred.
        return NULL;
    }

    // Check if there is room for the null terminator.
    COS_ASSERT(formatted_length < SIZE_MAX,
               "The formatted length should be less than SIZE_MAX");

    const size_t buffer_size = formatted_length + 1;
    char * const buffer = malloc(buffer_size * sizeof(char));
    if (!buffer) {
        return NULL;
    }

    // Consume the original variadic args because the caller expects us to.
    const int num_written = vsnprintf(buffer,
                                      buffer_size,
                                      format,
                                      args);
    if (num_written < 0) {
        // An error occurred.
        free(buffer);
        return NULL;
    }

    COS_ASSERT(num_written == buffer_size,
               "The number of characters written (%d) should be equal to the buffer size (%d)",
               num_written,
               buffer_size);

    return buffer;
}
