/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/io/string-support.h"

#include "common/Assert.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

void
cos_strlcpy(char *dest,
            size_t dest_size,
            const char *src)
{
    COS_PARAMETER_ASSERT(dest != NULL);
    COS_PARAMETER_ASSERT(src != NULL);

    strlcpy(dest, src, dest_size);
}

char *
cos_asprintf(const char *fmt, ...)
{
    COS_PARAMETER_ASSERT(fmt != NULL);

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
    // Determine the length of the formatted string, excluding the nul-terminator.
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
    COS_ASSERT(((size_t)formatted_length) < SIZE_MAX,
               "The formatted length should be less than SIZE_MAX");

    const size_t buffer_size = ((size_t)formatted_length) + 1;
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

    COS_ASSERT(((size_t)num_written) == buffer_size,
               "The number of characters written (%d) should be equal to the buffer size (%zu)",
               num_written,
               buffer_size);

    return buffer;
}

char *
cos_strdup(const char *str)
{
    if (!str) {
        return NULL;
    }

    return cos_strndup(str,
                       strlen(str));
}

char *
cos_strndup(const char *str,
            size_t n)
{
    if (!str) {
        return NULL;
    }

    const size_t str_len = strlen(str);

    const size_t copy_len = (str_len < n) ? str_len : n;

    char * const str_copy = malloc((copy_len + 1) * sizeof(char));
    if (!str_copy) {
        return NULL;
    }

    strncpy(str_copy,
            str,
            copy_len);

    str_copy[copy_len] = '\0';

    return str_copy;
}

COS_ASSUME_NONNULL_END
