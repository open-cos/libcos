//
// Created by david on 12/06/23.
//

#ifndef LIBCOS_COS_DEFINES_H
#define LIBCOS_COS_DEFINES_H

#if defined(__has_attribute)
#define COS_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#define COS_HAS_ATTRIBUTE(x) 0
#endif

/**
 * @def COS_FORMAT_PRINTF(format_index, args_index)
 *
 * @brief Marks a function as printf-like.
 *
 * @param format_index The index of the format string argument.
 *
 * @param args_index The index of the first variadic argument.
 * For functions that do not take a variable number of arguments, this should be 0.
 *
 * @note The indices start at 1.
 */
#if COS_HAS_ATTRIBUTE(format)
#define COS_FORMAT_PRINTF(format_index, args_index) __attribute__((format(printf, format_index, args_index)))
#else
#define COS_FORMAT_PRINTF(format_index, args_index)
#endif

/**
 * @def COS_NORETURN
 *
 * @brief Marks a function as not returning.
 */
#if COS_HAS_ATTRIBUTE(noreturn)
#define COS_NORETURN __attribute__((noreturn))
#else
#define COS_NORETURN
#endif

#endif /* LIBCOS_COS_DEFINES_H */
