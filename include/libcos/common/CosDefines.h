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

/**
 * @def COS_INLINE
 *
 * @brief Marks a function as inline.
 */
#if defined(_MSC_VER)
#define COS_STATIC_INLINE static __inline
#else
#define COS_STATIC_INLINE static inline
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

#if defined(__has_feature)
#define COS_HAS_FEATURE(x) __has_feature(x)
#else
#define COS_HAS_FEATURE(x) 0
#endif

#if defined(__has_extension)
#define COS_HAS_EXTENSION(x) __has_extension(x)
#else
#define COS_HAS_EXTENSION(x) 0
#endif

#if defined(__GNUC__) && !defined(__clang__)

#define COS_GCC_VERSION_AT_LEAST(major, minor, patchlevel) \
    ((__GNUC__ > (major)) ||                               \
     ((__GNUC__ == (major)) && COS_GCC_VERSION_AT_LEAST_MINOR_(minor, patchlevel)))

#if (___GNUC__ >= 2) && defined(__GNUC_MINOR__)
#define COS_GCC_VERSION_AT_LEAST_MINOR_(minor, patchlevel) \
    ((__GNUC_MINOR__ > (minor)) ||                         \
     ((__GNUC_MINOR__ == (minor)) && COS_GCC_VERSION_AT_LEAST_PATCHLEVEL_(patchlevel)))
#else
#define COS_GCC_VERSION_AT_LEAST_MINOR_(minor, patchlevel) (1)
#endif

#if (___GNUC__ >= 3) && defined(__GNUC_PATCHLEVEL__)
#define COS_GCC_VERSION_AT_LEAST_PATCHLEVEL_(patchlevel) \
    (__GNUC_PATCHLEVEL__ >= (patchlevel))
#else
#define COS_GCC_VERSION_AT_LEAST_PATCHLEVEL_(patchlevel) (1)
#endif

#else

#define COS_GCC_VERSION_AT_LEAST(major, minor, patchlevel) (0)

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
 * @def COS_ANALYZER_NORETURN
 *
 * @brief Marks a function as not returning for the static analyzer.
 */
#if COS_HAS_FEATURE(attribute_analyzer_noreturn)
#define COS_ANALYZER_NORETURN __attribute__((analyzer_noreturn))
#else
#define COS_ANALYZER_NORETURN
#endif

/**
 * @def COS_ATTR_NONSTRING
 *
 * @brief Marks a "string-like" variable as not being a NUL-terminated string.
 */
#if COS_HAS_ATTRIBUTE(nonstring)
#define COS_ATTR_NONSTRING __attribute__((nonstring))
#else
#define COS_ATTR_NONSTRING
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
 * @def COS_ATTR_UNUSED
 *
 * @brief Marks a variable as unused.
 */
#if COS_HAS_ATTRIBUTE(unused)
#define COS_ATTR_UNUSED __attribute__((unused))
#else
#define COS_ATTR_UNUSED
#endif

/**
 * @def COS_ATTR_PURE
 *
 * @brief Marks a function as returning a value that depends only on its arguments.
 */
#if COS_HAS_ATTRIBUTE(pure)
#define COS_ATTR_PURE __attribute__((pure))
#else
#define COS_ATTR_PURE
#endif

/**
 * @def COS_ATTR_MALLOC
 *
 * @brief Marks a function as returning a pointer to memory that should be freed.
 */
#if COS_HAS_ATTRIBUTE(malloc)
#define COS_ATTR_MALLOC __attribute__((malloc))

#if COS_GCC_VERSION_AT_LEAST(11, 1, 0)
#define COS_ATTR_MALLOC_DEALLOC(deallocator) __attribute__((malloc(deallocator)))
#define COS_ATTR_MALLOC_DEALLOC_INDEX(deallocator, ptr_index) __attribute__((malloc(deallocator, ptr_index)))
#else
#define COS_ATTR_MALLOC_DEALLOC(deallocator)
#define COS_ATTR_MALLOC_DEALLOC_INDEX(deallocator, ptr_index)
#endif

#else
#define COS_ATTR_MALLOC
#define COS_ATTR_MALLOC_DEALLOC(deallocator)
#define COS_ATTR_MALLOC_DEALLOC_INDEX(deallocator, ptr_index)
#endif

#define COS_MALLOC_LIKE COS_ATTR_MALLOC

#define COS_DEALLOC_FUNC(...) COS_GET_DEALLOC_FUNC_MACRO_(__VA_ARGS__,                   \
                                                          COS_ATTR_MALLOC_DEALLOC_INDEX, \
                                                          COS_ATTR_MALLOC_DEALLOC)(__VA_ARGS__)

#define COS_GET_DEALLOC_FUNC_MACRO_(_1, _2, NAME, ...) NAME

/**
 * @def COS_ATTR_ALLOC_SIZE(size_index)
 * @def COS_ATTR_ALLOC_SIZES(size_index1, size_index2)
 *
 * @brief Marks a function as returning a pointer to memory that should be freed,
 * and specifies the size of the allocated memory.
 */
#if COS_HAS_ATTRIBUTE(alloc_size)
#define COS_ATTR_ALLOC_SIZE(size_index) __attribute__((alloc_size(size_index)))
#define COS_ATTR_ALLOC_SIZES(size_index1, size_index2) __attribute__((alloc_size(size_index1, size_index2)))
#else
#define COS_ATTR_ALLOC_SIZE(size_index)
#define COS_ATTR_ALLOC_SIZES(size_index1, size_index2)
#endif

/**
 * Ownership attributes.
 */

#if COS_HAS_ATTRIBUTE(ownership_returns)
#define COS_ATTR_OWNERSHIP_RETURNS(module) __attribute__((ownership_returns(module)))
#define COS_ATTR_OWNERSHIP_RETURNS_SIZE(module, size_index) __attribute__((ownership_returns(module, size_index)))
#else
#define COS_ATTR_OWNERSHIP_RETURNS(module)
#define COS_ATTR_OWNERSHIP_RETURNS_SIZE(module, size_index)
#endif

#if COS_HAS_ATTRIBUTE(ownership_holds)
#define COS_ATTR_OWNERSHIP_HOLDS(module, ptr_index) __attribute__((ownership_holds(module, ptr_index)))
#else
#define COS_ATTR_OWNERSHIP_HOLDS(module, ptr_index)
#endif

#if COS_HAS_ATTRIBUTE(ownership_takes)
#define COS_ATTR_OWNERSHIP_TAKES(module, ...) __attribute__((ownership_takes(module, __VA_ARGS__)))
#else
#define COS_ATTR_OWNERSHIP_TAKES(module, ...)
#endif

#define COS_MALLOC_OWNERSHIP_RETURNS COS_ATTR_OWNERSHIP_RETURNS(malloc)
#define COS_MALLOC_OWNERSHIP_RETURNS_SIZE(size_index) COS_ATTR_OWNERSHIP_RETURNS_SIZE(malloc, size_index)
#define COS_MALLOC_OWNERSHIP_HOLDS(ptr_index) COS_ATTR_OWNERSHIP_HOLDS(malloc, ptr_index)
#define COS_MALLOC_OWNERSHIP_TAKES(...) COS_ATTR_OWNERSHIP_TAKES(malloc, __VA_ARGS__)

/**
 * @brief Marks a function as returning ownership of a pointer.
 */
#define COS_OWNERSHIP_RETURNS    \
    COS_MALLOC_OWNERSHIP_RETURNS \
    COS_ATTR_MALLOC              \
        COS_WARN_UNUSED_RESULT

/**
 * @brief Marks a function as holding ownership of a pointer.
 */
#define COS_OWNERSHIP_HOLDS(ptr_index) COS_MALLOC_OWNERSHIP_HOLDS(ptr_index)

/**
 * @brief Marks a function as taking ownership of a pointer or pointers.
 */
#define COS_OWNERSHIP_TAKES(...) COS_MALLOC_OWNERSHIP_TAKES(__VA_ARGS__)

/**
 * Pointer access mode attributes.
 */

#if COS_HAS_ATTRIBUTE(access)
#define COS_ATTR_ACCESS(access_mode, ref_index) __attribute__((access(access_mode, ref_index)))
#define COS_ATTR_ACCESS_SIZE(access_mode, ref_index, size_index) __attribute__((access(access_mode, ref_index, size_index)))
#else
#define COS_ATTR_ACCESS(access_mode, ref_index)
#define COS_ATTR_ACCESS_SIZE(access_mode, ref_index, size_index)
#endif

#define COS_ATTR_ACCESS_READ_ONLY(ref_index) COS_ATTR_ACCESS(read_only, ref_index)
#define COS_ATTR_ACCESS_READ_WRITE(ref_index) COS_ATTR_ACCESS(read_write, ref_index)
#define COS_ATTR_ACCESS_WRITE_ONLY(ref_index) COS_ATTR_ACCESS(write_only, ref_index)
#define COS_ATTR_ACCESS_NONE(ref_index) COS_ATTR_ACCESS(none, ref_index)

#define COS_ATTR_ACCESS_READ_ONLY_SIZE(ref_index, size_index) COS_ATTR_ACCESS_SIZE(read_only, ref_index, size_index)
#define COS_ATTR_ACCESS_READ_WRITE_SIZE(ref_index, size_index) COS_ATTR_ACCESS_SIZE(read_write, ref_index, size_index)
#define COS_ATTR_ACCESS_WRITE_ONLY_SIZE(ref_index, size_index) COS_ATTR_ACCESS_SIZE(write_only, ref_index, size_index)

#if COS_HAS_ATTRIBUTE(fallthrough)
#define COS_ATTR_FALLTHROUGH __attribute__((fallthrough))
#else
#define COS_ATTR_FALLTHROUGH
#endif

#if COS_HAS_ATTRIBUTE(diagnose_if)
#define COS_ATTR_DIAGNOSE_IF(condition, message, type) __attribute__((diagnose_if(condition, message, type)))
#else
#define COS_ATTR_DIAGNOSE_IF(condition, message, type)
#endif

#define COS_ATTR_DIAGNOSE_WARNING(condition, message) COS_ATTR_DIAGNOSE_IF(condition, message, "warning")
#define COS_ATTR_DIAGNOSE_ERROR(condition, message) COS_ATTR_DIAGNOSE_IF(condition, message, "error")

/*
 * Allocator.
 */

/**
 * @brief Marks a function as an allocator.
 */
#define COS_ALLOCATOR_FUNC       \
    COS_ATTR_MALLOC              \
    COS_MALLOC_OWNERSHIP_RETURNS \
    COS_WARN_UNUSED_RESULT

/**
 * @brief Marks a function as an allocator that returns a pointer to memory of the specified size.
 *
 * @param size_index The index of the size argument.
 */
#define COS_ALLOCATOR_FUNC_SIZE(size_index)       \
    COS_ATTR_MALLOC                               \
    COS_ATTR_ALLOC_SIZE(size_index)               \
    COS_MALLOC_OWNERSHIP_RETURNS_SIZE(size_index) \
    COS_WARN_UNUSED_RESULT

/**
 * @brief Marks a function as the matching allocator of the specified deallocator.
 *
 * @param deallocator The deallocator function.
 *
 * @note The deallocator function must be declared before it can be referenced by this macro.
 */
#define COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(deallocator) \
    COS_ATTR_MALLOC_DEALLOC(deallocator)

#define COS_ALLOCATOR_FUNC_MATCHED_DEALLOC_INDEX(deallocator, ptr_index) \
    COS_ATTR_MALLOC_DEALLOC_INDEX(deallocator, ptr_index)

/**
 * @brief Marks a function as a deallocator.
 *
 * @see COS_DEALLOCATOR_FUNC_INDEX
 */
#define COS_DEALLOCATOR_FUNC COS_DEALLOCATOR_FUNC_INDEX(1)

/**
 * @brief Marks a function as a deallocator that takes ownership of the pointer.
 *
 * Read-write access to the pointer argument is required.
 *
 * @param ptr_index The index of the pointer argument.
 */
#define COS_DEALLOCATOR_FUNC_INDEX(ptr_index) \
    COS_MALLOC_OWNERSHIP_TAKES(ptr_index)     \
    COS_ATTR_ACCESS_READ_WRITE(ptr_index)

/**
 * @brief Marks a function as a reallocator.
 *
 * @param ptr_index The index of the pointer argument.
 * @param size_index The index of the size argument.
 */
#define COS_REALLOCATOR_FUNC(ptr_index, size_index) \
    COS_ATTR_ALLOC_SIZE(size_index)                 \
    COS_DEALLOCATOR_FUNC_INDEX(ptr_index)           \
    COS_MALLOC_OWNERSHIP_RETURNS_SIZE(size_index)   \
    COS_WARN_UNUSED_RESULT

/**
 * Precondition.
 */

#define COS_PRECONDITION(condition) COS_ATTR_DIAGNOSE_WARNING(!(condition), "Precondition not satisfied: " #condition)

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

#define COS_nonnull_cast(x) ((__typeof__(*(x)) * _Nonnull)(x))
#else
#define COS_Nullable /* nothing */
#define COS_Nonnull  /* nothing */

#define COS_nonnull_cast(x) (x)
#endif

#if COS_HAS_EXTENSION(assume_nonnull)
#define COS_ASSUME_NONNULL_BEGIN _Pragma("clang assume_nonnull begin")
#define COS_ASSUME_NONNULL_END _Pragma("clang assume_nonnull end")
#else
#define COS_ASSUME_NONNULL_BEGIN
#define COS_ASSUME_NONNULL_END
#endif

/*
 * Enumerations
 */

#if COS_HAS_ATTRIBUTE(enum_extensibility)
#define COS_ATTR_ENUM_EXTENSIBILITY(type) __attribute__((enum_extensibility(type)))
#else
#define COS_ATTR_ENUM_EXTENSIBILITY(type)
#endif

/**
 * @brief Marks an enumeration as open.
 *
 * An open enumeration can have values that are not explicitly defined.
 */
#define COS_OPEN_ENUM COS_ATTR_ENUM_EXTENSIBILITY(open)

/**
 * @brief Marks an enumeration as closed.
 *
 * A closed enumeration can only have values that are explicitly defined.
 */
#define COS_CLOSED_ENUM COS_ATTR_ENUM_EXTENSIBILITY(closed)

/**
 * @def COS_ATTR_FLAG_ENUM
 *
 * @brief Marks an enumeration as a flag enum.
 */
#if COS_HAS_ATTRIBUTE(flag_enum)
#define COS_ATTR_FLAG_ENUM __attribute__((flag_enum))
#else
#define COS_ATTR_FLAG_ENUM
#endif

#endif /* LIBCOS_COS_DEFINES_H */
