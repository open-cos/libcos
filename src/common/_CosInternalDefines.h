/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_INTERNAL_DEFINES_H
#define LIBCOS_COMMON_COS_INTERNAL_DEFINES_H

#if defined(__linux__)
    // Required to get the glibc version definitions.
    #include <features.h>
#endif

/**
 * @def COS_HAS_GLIBC
 *
 * @brief Whether the GNU C Library is available.
 */
#if defined(__GLIBC__) && defined(__GLIBC_MINOR__)
    #define COS_HAS_GLIBC 1
#else
    #define COS_HAS_GLIBC 0
#endif

/**
 * @def COS_GLIBC_PREREQ(major_version, minor_version)
 *
 * @brief Checks if the GNU C Library version is at least the specified version.
 *
 * @param major_version The major version number.
 * @param minor_version The minor version number.
 */
#if COS_HAS_GLIBC
    #define COS_GLIBC_PREREQ(major_version, minor_version) \
        ((__GLIBC__ > (major_version)) ||                  \
         ((__GLIBC__ == (major_version)) && (__GLIBC_MINOR__ >= (minor_version))))
#else
    #define COS_GLIBC_PREREQ(major_version, minor_version) (0)
#endif

#endif /* LIBCOS_COMMON_COS_INTERNAL_DEFINES_H */
