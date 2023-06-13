//
// Created by david on 10/06/23.
//

#ifndef LIBCOS_FORMAT_H
#define LIBCOS_FORMAT_H

#include <libcos/common/CosAttributes.h>

#include <stdarg.h>

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

#endif //LIBCOS_FORMAT_H
