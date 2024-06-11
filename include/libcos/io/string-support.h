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
 * @brief Copies a nul-terminated string.
 *
 * @param dest The destination buffer.
 * @param dest_size The size of the @p dest buffer.
 * @param src The nul-terminated source string.
 *
 * @return The number of characters copied, excluding the nul-terminator.
 */
size_t
cos_strlcpy(char *dest,
            size_t dest_size,
            const char *src)
    COS_ATTR_ACCESS_WRITE_ONLY_SIZE(1, 2)
    COS_ATTR_ACCESS_READ_ONLY(3)
    COS_ATTR_NULL_TERMINATED_STRING_ARG(3);

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
