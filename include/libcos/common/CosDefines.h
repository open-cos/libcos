//
// Created by david on 12/06/23.
//

#ifndef LIBCOS_COS_DEFINES_H
#define LIBCOS_COS_DEFINES_H

#if defined(__cplusplus)
#define COS_DECLS_BEGIN extern "C" {
#define COS_DECLS_END }
#else
#define COS_DECLS_BEGIN
#define COS_DECLS_END
#endif

#if defined(__has_attribute)
#define COS_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#define COS_HAS_ATTRIBUTE(x) 0
#endif

#if defined(__has_builtin)
#define COS_HAS_BUILTIN(x) __has_builtin(x)
#else
#define COS_HAS_BUILTIN(x) 0
#endif

#if defined(__has_extension)
#define COS_HAS_EXTENSION(x) __has_extension(x)
#else
#define COS_HAS_EXTENSION(x) 0
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

/**
 * @def COS_NONSTRING
 *
 * @brief Marks a "string-like" variable as not being a NUL-terminated string.
 */
#if COS_HAS_ATTRIBUTE(nonstring)
#define COS_NONSTRING __attribute__((nonstring))
#else
#define COS_NONSTRING
#endif

/**
 * @def COS_WARN_UNUSED_RESULT
 *
 * @brief Marks a function as returning a value that should be used.
 */
#if COS_HAS_ATTRIBUTE(warn_unused_result)
#define COS_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define COS_WARN_UNUSED_RESULT
#endif

/**
 * @def COS_ATTR_MALLOC
 *
 * @brief Marks a function as returning a pointer to memory that should be freed.
 */
#if COS_HAS_ATTRIBUTE(malloc)
#define COS_ATTR_MALLOC __attribute__((malloc))
#else
#define COS_ATTR_MALLOC
#endif

#if COS_HAS_ATTRIBUTE(access)
#define COS_ATTR_ACCESS(access_mode, ref_index) __attribute__((access(access_mode, ref_index)))
#define COS_ATTR_ACCESS_SIZE(access_mode, ref_index, size_index) __attribute__((access(access_mode, ref_index, size_index)))
#else
#define COS_ATTR_ACCESS(access_mode, ref_index)
#define COS_ATTR_ACCESS_SIZE(access_mode, ref_index, size_index)
#endif

#define COS_ATTR_ACCESS_READONLY(ref_index) COS_ATTR_ACCESS(read_only, ref_index)
#define COS_ATTR_ACCESS_READ_WRITE(ref_index) COS_ATTR_ACCESS(read_write, ref_index)
#define COS_ATTR_ACCESS_WRITE_ONLY(ref_index) COS_ATTR_ACCESS(write_only, ref_index)
#define COS_ATTR_ACCESS_NONE(ref_index) COS_ATTR_ACCESS(none, ref_index)

#if COS_HAS_ATTRIBUTE(fallthrough)
#define COS_ATTR_FALLTHROUGH __attribute__((fallthrough))
#else
#define COS_ATTR_FALLTHROUGH
#endif

#if COS_HAS_BUILTIN(__builtin_expect)
#define COS_LIKELY(x) __builtin_expect(!!(x), 1)
#define COS_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define COS_LIKELY(x) (x)
#define COS_UNLIKELY(x) (x)
#endif

#if COS_HAS_EXTENSION(nullability)
#define COS_Nullable _Nullable
#define COS_Nonnull _Nonnull
#else
#define COS_Nullable /* nothing */
#define COS_Nonnull  /* nothing */
#endif

#if COS_HAS_EXTENSION(assume_nonnull)
#define COS_ASSUME_NONNULL_BEGIN _Pragma("clang assume_nonnull begin")
#define COS_ASSUME_NONNULL_END _Pragma("clang assume_nonnull end")
#else
#define COS_ASSUME_NONNULL_BEGIN
#define COS_ASSUME_NONNULL_END
#endif

#endif /* LIBCOS_COS_DEFINES_H */
