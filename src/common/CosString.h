//
// Created by david on 24/05/23.
//

#ifndef LIBCOS_COS_STRING_H
#define LIBCOS_COS_STRING_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * A nul-terminated character array.
 */
struct CosString {
    /**
     * The nul-terminated character array.
     */
    char * COS_Nullable data;

    /**
     * The number of characters in the string, including the nul-terminator.
     */
    size_t size;
};

/**
 * Allocates a new empty string.
 *
 * @return The new string, or @c NULL if memory allocation failed.
 *
 * @note The returned string must be freed with @c cos_string_free().
 * @see cos_string_free()
 */
CosString * COS_Nullable
cos_string_alloc(void)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

/**
 * Allocates a new string with the given C-string.
 *
 * @param data The null-terminated C-string to copy.
 * @return The new string, or @c NULL if memory allocation failed.
 *
 * @note The returned string must be freed with @c cos_string_free().
 * @see cos_string_free()
 */
CosString * COS_Nullable
cos_string_alloc_with_str(const char *str)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT
    COS_ATTR_ACCESS_READONLY(1);

/**
 * Allocates a new string with the given C-string.
 *
 * @param data The C-string to copy.
 * @param n The number of characters to copy.
 * @return The new string, or @c NULL if memory allocation failed.
 *
 * @note The returned string must be freed with @c cos_string_free().
 * @see cos_string_free()
 */
CosString * COS_Nullable
cos_string_alloc_with_strn(const char *str,
                           size_t n)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT
    COS_ATTR_ACCESS_READONLY_SIZE(1, 2);

/**
 * Frees a string.
 *
 * @param string The string to free.
 *
 * @note The string must have been allocated with @c cos_string_alloc(), etc.
 */
void
cos_string_free(CosString *string);

/**
 * Creates a copy of the string.
 *
 * @param string The string to copy.
 *
 * @return The new string, or @c NULL if memory allocation failed.
 *
 * @note The returned string must be freed with @c cos_string_free().
 */
CosString * COS_Nullable
cos_string_copy(const CosString *string)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT
    COS_ATTR_ACCESS_READONLY(1);

#pragma mark - String Reference

/**
 * A reference to a nul-terminated character array.
 */
struct CosStringRef {
    /**
     * The nul-terminated character array.
     */
    const char * COS_Nullable data;

    /**
     * The number of characters in the string, including the nul-terminator.
     */
    size_t count;
};

/**
 * Creates a read-only string reference from the given string.
 *
 * @param string The string.
 *
 * @return The string reference.
 *
 * @note The string reference is valid only as long as the string is valid.
 */
CosStringRef
cos_string_make_ref(const CosString *string)
    COS_ATTR_ACCESS_READONLY(1);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_STRING_H */
