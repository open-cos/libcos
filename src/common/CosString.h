//
// Created by david on 24/05/23.
//

#ifndef LIBCOS_COS_STRING_H
#define LIBCOS_COS_STRING_H

#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/**
 * Allocates a new empty string.
 *
 * @return The new string, or @c NULL if memory allocation failed.
 *
 * @note The returned string must be freed with @c cos_string_free().
 * @see cos_string_free()
 */
CosString *
cos_string_alloc(void);

/**
 * Allocates a new string with the given C-string.
 *
 * @param str The null-terminated C-string to copy.
 * @return The new string, or @c NULL if memory allocation failed.
 *
 * @note The returned string must be freed with @c cos_string_free().
 * @see cos_string_free()
 */
CosString *
cos_string_alloc_with_str(const char *str);

/**
 * Allocates a new string with the given C-string.
 *
 * @param str The C-string to copy.
 * @param n The number of characters to copy.
 * @return The new string, or @c NULL if memory allocation failed.
 *
 * @note The returned string must be freed with @c cos_string_free().
 * @see cos_string_free()
 */
CosString *
cos_string_alloc_with_strn(const char *str,
                           size_t n);

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
CosString *
cos_string_copy(const CosString *string);

/**
 * Returns the length of the string.
 *
 * @param string The string.
 * @return The number of characters in the string, excluding the nul-terminator.
 */
size_t
cos_string_get_length(const CosString *string);

/**
 * Returns the string's str.
 *
 * @param string The string.
 * @return A pointer to the nul-terminated character array.
 */
const char *
cos_string_get_str(const CosString *string);

/**
 * A reference to a nul-terminated character array.
 */
struct CosStringRef {
    /**
     * The nul-terminated character array.
     */
    const char *data;

    /**
     * The number of characters in the string, including the nul-terminator.
     */
    size_t count;
};

CosStringRef
cos_string_get_ref(const CosString *string);

#endif /* LIBCOS_COS_STRING_H */
