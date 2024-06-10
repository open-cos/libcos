/*
 * Copyright (c) 2024 OpenCOS.
 */

//#define _GNU_SOURCE

#include "common/cos-strerror.h"

#include "common/Assert.h"
#include "common/_CosInternalDefines.h"
#include "io/string-support.h"

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define COS_STRERROR_BUFFER_LENGTH 1024
#define COS_STRERROR_MAX_LENGTH 4096

#if (defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200112L) || defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 600)) && !defined(_GNU_SOURCE)
    // The XSI-compliant version of strerror_r() is available.
    #define COS_HAS_XSI_STRERROR_R 1
    #define COS_HAS_GNU_STRERROR_R 0

    // The XSI-compliant strerror_r() is only POSIX-compliant since glibc 2.13.
    #if COS_HAS_GLIBC && !COS_GLIBC_PREREQ(2, 13)
        #define COS_HAS_PRE_COMPLIANT_XSI_STRERROR_R 1
    #endif
#elif defined(_GNU_SOURCE)
    #define COS_HAS_XSI_STRERROR_R 0
    #if COS_GLIBC_PREREQ(2, 0)
        // The GNU-specific version of strerror_r() is available.
        #define COS_HAS_GNU_STRERROR_R 1
    #else
        #define COS_HAS_GNU_STRERROR_R 0
    #endif
#else
    #define COS_HAS_XSI_STRERROR_R 0
    #define COS_HAS_GNU_STRERROR_R 0
#endif

#ifndef COS_HAS_PRE_COMPLIANT_XSI_STRERROR_R
    #define COS_HAS_PRE_COMPLIANT_XSI_STRERROR_R 0
#endif

COS_ASSUME_NONNULL_BEGIN

#if COS_HAS_GNU_STRERROR_R

static inline int
cos_strerror_r_func_(int errnum,
                     char *buffer,
                     size_t buffer_size)
    COS_OWNERSHIP_RETURNS
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(free)
{
    errno = 0;
    const char * const strerror_result = strerror_r(errnum,
                                                    buffer,
                                                    buffer_size);
    const int strerror_errno = errno;
    if (strerror_errno != 0) {
        return strerror_errno;
    }

    if (strerror_result != buffer) {
        // A pointer to some (immutable) static string was returned.
        // NOTE: The GNU-specific strerror_r() function returns nul-terminated strings.
        cos_strlcpy(buffer,
                    buffer_size,
                    strerror_result);
    }

    return 0;
}

#elif COS_HAS_XSI_STRERROR_R

// Provide a compliant wrapper for the pre-compliant XSI strerror_r() function.
static inline int
cos_strerror_r_func_(int errnum,
                     char *buffer,
                     size_t buffer_size)
{
    const int strerror_result = strerror_r(errnum,
                                           buffer,
                                           buffer_size);

    #if COS_HAS_PRE_COMPLIANT_XSI_STRERROR_R
    /*
     * The (glibc) pre-compliant XSI strerror_r() function returns 0 on success,
     * or -1 is returned and errno is set to indicate the error.
     */
    if (strerror_result != 0) {
        COS_ASSERT(strerror_result == -1, "Expected a strerror_r() return value of -1");
        const int strerror_errno = errno;
        return strerror_errno;
    }
    #endif

    return strerror_result;
}

#endif

static inline char * COS_Nullable
cos_strerror_r_(int errnum)
    COS_OWNERSHIP_RETURNS
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(free)
{
    char *buffer = NULL;
    size_t buffer_size = COS_STRERROR_BUFFER_LENGTH;

    while (buffer_size < COS_STRERROR_MAX_LENGTH) {
        char * const new_buffer = realloc(buffer,
                                          buffer_size * sizeof(char));
        if (!new_buffer) {
            goto failure;
        }
        buffer = new_buffer;

        const int strerror_result = cos_strerror_r_func_(errnum,
                                                         buffer,
                                                         buffer_size);
        if (strerror_result == 0) {
            return buffer;
        }
        else if (strerror_result == ERANGE) {
            // The buffer is too small.
            buffer_size *= 2;
        }
        else {
            goto failure;
        }
    }

    return buffer;

failure:
    if (buffer) {
        free(buffer);
    }
    return NULL;
}

char *
cos_strerror(int errnum)
{
    return cos_strerror_r_(errnum);
}

COS_ASSUME_NONNULL_END
