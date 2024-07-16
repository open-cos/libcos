/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_SCANNER_H
#define LIBCOS_COMMON_COS_SCANNER_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief Allocates a new scanner.
 *
 * @param buffer The input buffer, or @c NULL.
 * @param buffer_size The size of the input buffer, or @c 0.
 *
 * @return The scanner, or @c NULL if memory allocation failed.
 */
CosScanner * COS_Nullable
cos_scanner_alloc(const char * COS_Nullable buffer,
                  size_t buffer_size)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

/**
 * @brief Frees a scanner.
 *
 * @param scanner The scanner.
 */
void
cos_scanner_free(CosScanner *scanner);

/**
 * @brief Sets the input for the scanner.
 *
 * @param scanner The scanner.
 * @param buffer The input buffer, or @c NULL to clear the input.
 * @param buffer_size The size of the input buffer, or @c 0.
 */
void
cos_scanner_set_input(CosScanner *scanner,
                      const char * COS_Nullable buffer,
                      size_t buffer_size);

/**
 * @brief Returns the current position of the scanner in the input.
 *
 * @param scanner The scanner.
 *
 * @return The current position of the scanner in the input.
 */
size_t
cos_scanner_get_position(const CosScanner *scanner);

/**
 * @brief Returns whether the scanner is at the end of the input.
 *
 * @param scanner The scanner.
 *
 * @return Whether the scanner is at the end of the input.
 */
bool
cos_scanner_is_at_end(const CosScanner *scanner);

/**
 * @brief Resets the scanner to the beginning of the input.
 *
 * @param scanner The scanner.
 */
void
cos_scanner_reset(CosScanner *scanner);

// MARK: - Reading

/**
 * @brief Reads a character from the input.
 *
 * @param scanner The scanner.
 * @param out_char The output character.
 *
 * @return Whether a character was read.
 */
bool
cos_scanner_read_char(CosScanner *scanner,
                      char *out_char)
    COS_ATTR_ACCESS_WRITE_ONLY(2);

/**
 * @brief Reads a character from the input and matches it against an expected character.
 *
 * @param scanner The scanner.
 * @param expected_char The expected character.
 *
 * @return @c true if the character was read and matched the expected character,
 * @c false otherwise.
 */
bool
cos_scanner_match_char(CosScanner *scanner,
                       char expected_char);

/**
 * @brief Reads an unsigned integer from the input.
 *
 * @param scanner The scanner.
 * @param max_num_digits The maximum number of digits to read, or 0 for no limit.
 * @param out_value The output value.
 *
 * @return The number of digits read, or 0 if no digits were read.
 */
size_t
cos_scanner_read_unsigned(CosScanner *scanner,
                          size_t max_num_digits,
                          unsigned long *out_value,
                          CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

/**
 * @brief Skips whitespace in the input.
 *
 * @param scanner The scanner.
 *
 * @return The number of whitespace characters skipped.
 */
size_t
cos_scanner_skip_whitespace(CosScanner *scanner);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_SCANNER_H */
