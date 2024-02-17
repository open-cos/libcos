/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_STRING_SUPPORT_H
#define LIBCOS_STRING_SUPPORT_H

#include <libcos/common/CosDefines.h>

#include <stdarg.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief Formats a string.
 *
 * @param fmt The format string.
 * @param ... The format arguments.
 *
 * @return The formatted string, or @c NULL if an error occurred.
 */
char * COS_Nullable
cos_asprintf(const char *fmt, ...)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT
    COS_FORMAT_PRINTF(1, 2);

/**
 * @brief Formats a string.
 *
 * @param format The format string.
 * @param args The format arguments.
 *
 * @return The formatted string, or @c NULL if an error occurred.
 */
char * COS_Nullable
cos_vasprintf(const char *format,
              va_list args)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT
    COS_FORMAT_PRINTF(1, 0);

/**
 * @brief Duplicates a string.
 *
 * @param str The string to duplicate.
 *
 * @return The duplicated string, or @c NULL if an error occurred.
 */
char * COS_Nullable
cos_strdup(const char *str)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

/**
 * @brief Duplicates a string.
 *
 * @param str The string to duplicate.
 * @param n The number of characters to duplicate.
 *
 * @return  The duplicated string, or @c NULL if an error occurred.
 */
char * COS_Nullable
cos_strndup(const char *str,
            size_t n)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif //LIBCOS_STRING_SUPPORT_H
