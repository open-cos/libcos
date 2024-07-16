/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_STRING_H
#define LIBCOS_COMMON_COS_STRING_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * Allocates a new empty string.
 *
 * @param capacity_hint A hint for the initial capacity of the string.
 *
 * @return The new string, or @c NULL if memory allocation failed.
 *
 * @note The returned string must be freed with @c cos_string_free().
 * @see cos_string_free()
 */
CosString * COS_Nullable
cos_string_alloc(size_t capacity_hint)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

/**
 * Allocates a new string with the given C-string.
 *
 * @param str The null-terminated C-string to copy.
 * @return The new string, or @c NULL if memory allocation failed.
 *
 * @note The returned string must be freed with @c cos_string_free().
 * @see cos_string_free()
 */
CosString * COS_Nullable
cos_string_alloc_with_str(const char *str)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT
    COS_ATTR_ACCESS_READ_ONLY(1);

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
CosString * COS_Nullable
cos_string_alloc_with_strn(const char *str, size_t n)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT COS_ATTR_ACCESS_READ_ONLY_SIZE(1, 2);

/**
 * Frees a string.
 *
 * @param string The string to free.
 *
 * @note The string must have been allocated with @c cos_string_alloc(), etc.
 */
void
cos_string_free(CosString *string);

// MARK: Initialization

/**
 * Initializes a string with the default capacity.
 *
 * @param string The string to initialize.
 *
 * @return @c true if initialization succeeded, @c false otherwise.
 */
bool
cos_string_init(CosString *string);

/**
 * Initializes a string with the given capacity.
 *
 * @param string The string to initialize.
 * @param capacity_hint A hint for the initial capacity of the string.
 *
 * @return @c true if initialization succeeded, @c false otherwise.
 */
bool
cos_string_init_capacity(CosString *string, size_t capacity_hint);

/**
 * @brief Gets the string's nul-terminated character array.
 *
 * @param string The string.
 *
 * @return The nul-terminated character array, or @c NULL if the string is empty.
 */
const char * COS_Nullable
cos_string_get_data(const CosString *string)
    COS_ATTR_ACCESS_READ_ONLY(1);

/**
 * @brief Gets the number of characters in the string, not including the nul-terminator.
 *
 * @param string The string.
 *
 * @return The number of characters in the string.
 */
size_t
cos_string_get_length(const CosString *string)
    COS_ATTR_ACCESS_READ_ONLY(1);

/**
 * @brief Gets the number of characters that can be stored in the string.
 *
 * @param string The string.
 *
 * @return The number of characters that can be stored in the string.
 */
size_t
cos_string_get_capacity(const CosString *string)
    COS_ATTR_ACCESS_READ_ONLY(1);

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
    COS_ATTR_ACCESS_READ_ONLY(1);

bool
cos_string_append_str(CosString *string, const char *str)
    COS_ATTR_ACCESS_READ_ONLY(2);

bool
cos_string_append_strn(CosString *string, const char *str, size_t n)
    COS_ATTR_ACCESS_READ_ONLY_SIZE(2, 3);

/**
 * Appends the given character to the string.
 *
 * @param string The string to be modified.
 * @param c The character to append.
 *
 * @return @c true if the character was appended, @c false otherwise.
 */
bool
cos_string_push_back(CosString *string, char c);

/** @{ **/

/**
 * @brief Returns the hash value of the string.
 *
 * @param string The string.
 *
 * @return The hash value of the string.
 */
size_t
cos_string_get_hash(const CosString *string)
    COS_ATTR_ACCESS_READ_ONLY(1);

/** @} */

// MARK: String Reference

/**
 * A reference to a read-only nul-terminated character array.
 */
struct CosStringRef {
    /**
     * The nul-terminated character array.
     */
    const char * COS_Nullable data;

    /**
     * The number of characters in the string, not including the nul-terminator.
     */
    size_t length;
};

/**
 * @def cos_string_ref_const(str)
 *
 * Creates a read-only string reference from the given C-string literal.
 */
#define cos_string_ref_const(str)                          \
    (CosStringRef)                                         \
    {                                                      \
        .data = (str),                                     \
        .length = ((str != NULL) ? (sizeof(str) - 1) : 0), \
    }

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
cos_string_get_ref(const CosString *string)
    COS_ATTR_ACCESS_READ_ONLY(1);

/**
 * Creates a read-only string reference from the given C-string.
 *
 * @param str The C-string.
 *
 * @return The string reference.
 *
 * @note The string reference is valid only as long as the C-string is valid.
 * @see @c cos_string_ref_make()
 */
CosStringRef
cos_string_ref_from_str(const char *str)
    COS_ATTR_ACCESS_READ_ONLY(1);

/**
 * Creates a read-only string reference from the given C-string.
 *
 * @param str The C-string.
 * @param n The number of characters in the C-string.
 *
 * @return The string reference.
 *
 * @note The string reference is valid only as long as the C-string is valid.
 */
CosStringRef
cos_string_ref_make(const char *str, size_t n)
    COS_ATTR_ACCESS_READ_ONLY_SIZE(1, 2);

/**
 * Compares two string references for equality.
 *
 * @param lhs The first string reference.
 * @param rhs The second string reference.
 *
 * @return An integer less than, equal to, or greater than zero if @p lhs is less than,
 * equal to, or greater than @p rhs, respectively.
 */
int
cos_string_ref_cmp(CosStringRef lhs, CosStringRef rhs);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_STRING_H */
