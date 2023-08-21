//
// Created by david on 10/06/23.
//

#ifndef LIBCOS_STRING_SUPPORT_H
#define LIBCOS_STRING_SUPPORT_H

#include <libcos/common/CosDefines.h>

#include <stdarg.h>
#include <stddef.h>

/**
 * @brief Formats a string.
 *
 * @param fmt The format string.
 * @param ... The format arguments.
 *
 * @return The formatted string, or NULL if an error occurred.
 */
char *
cos_asprintf(const char *fmt, ...) COS_FORMAT_PRINTF(1, 2);

/**
 * @brief Formats a string.
 *
 * @param format The format string.
 * @param args The format arguments.
 *
 * @return The formatted string, or NULL if an error occurred.
 */
char *
cos_vasprintf(const char *format,
              va_list args) COS_FORMAT_PRINTF(1, 0);

/**
 * @brief Duplicates a string.
 *
 * @param str The string to duplicate.
 * @return The duplicated string, or NULL if an error occurred.
 */
char *
cos_strdup(const char *str);

/**
 * @brief Duplicates a string.
 *
 * @param str The string to duplicate.
 * @param n The number of characters to duplicate.
 * @return  The duplicated string, or NULL if an error occurred.
 */
char *
cos_strndup(const char *str,
            size_t n);

#endif //LIBCOS_STRING_SUPPORT_H
