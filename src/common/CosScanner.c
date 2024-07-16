/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/common/CosScanner.h"

#include "common/Assert.h"
#include "common/CharacterSet.h"
#include "libcos/common/CosError.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

struct CosScanner {
    const char * COS_Nullable buffer;
    size_t buffer_size;
    size_t position;
};

CosScanner *
cos_scanner_alloc(const char * COS_Nullable buffer,
                  size_t buffer_size)
{
    CosScanner *scanner = NULL;

    scanner = calloc(1, sizeof(CosScanner));
    if (!scanner) {
        return NULL;
    }

    scanner->buffer = buffer;
    scanner->buffer_size = buffer_size;

    return scanner;
}

void
cos_scanner_free(CosScanner *scanner)
{
    if (!scanner) {
        return;
    }

    free(scanner);
}

void
cos_scanner_set_input(CosScanner *scanner,
                      const char * COS_Nullable buffer,
                      size_t buffer_size)
{
    COS_PARAMETER_ASSERT(scanner != NULL);
    if (!scanner) {
        return;
    }

    scanner->buffer = buffer;
    scanner->buffer_size = buffer_size;
    scanner->position = 0;
}

size_t
cos_scanner_get_position(const CosScanner *scanner)
{
    COS_PARAMETER_ASSERT(scanner != NULL);
    if (!scanner) {
        return 0;
    }

    return scanner->position;
}

bool
cos_scanner_is_at_end(const CosScanner *scanner)
{
    COS_PARAMETER_ASSERT(scanner != NULL);
    if (!scanner) {
        return true;
    }

    return (scanner->position >= scanner->buffer_size);
}

void
cos_scanner_reset(CosScanner *scanner)
{
    COS_PARAMETER_ASSERT(scanner != NULL);
    if (!scanner) {
        return;
    }

    scanner->position = 0;
}

// MARK: - Reading

bool
cos_scanner_read_char(CosScanner *scanner,
                      char *out_char)
{
    COS_PARAMETER_ASSERT(scanner != NULL);
    COS_PARAMETER_ASSERT(out_char != NULL);
    if (!scanner || !out_char) {
        return false;
    }

    if (scanner->position >= scanner->buffer_size) {
        return false;
    }

    *out_char = scanner->buffer[scanner->position];
    scanner->position++;
    return true;
}

bool
cos_scanner_match_char(CosScanner *scanner,
                       char expected_char)
{
    COS_PARAMETER_ASSERT(scanner != NULL);
    if (!scanner) {
        return false;
    }

    if (scanner->position >= scanner->buffer_size) {
        return false;
    }

    const char c = scanner->buffer[scanner->position];
    if (c != expected_char) {
        return false;
    }

    scanner->position++;
    return true;
}

size_t
cos_scanner_read_unsigned(CosScanner *scanner,
                          size_t max_num_digits,
                          unsigned long *out_value,
                          CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(scanner != NULL);
    COS_PARAMETER_ASSERT(out_value != NULL);
    if (!scanner || !out_value) {
        return 0;
    }

    size_t length = 0;

    while ((scanner->position + length) < scanner->buffer_size &&
           (max_num_digits > 0 && length < max_num_digits)) {
        const char c = scanner->buffer[scanner->position];
        if (!cos_is_decimal_digit(c)) {
            break;
        }
        length++;
    }

    if (length == 0) {
        return 0;
    }

    // Copy the string to a new nul-terminated buffer.
    char *str_copy = malloc(length + 1);
    if (!str_copy) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to allocate memory for string copy."),
                            out_error);
        goto failure;
    }

    memcpy(str_copy, &(scanner->buffer[scanner->position]), length);
    str_copy[length] = '\0';

    // Advance the scanner's position.
    scanner->position += length;

    errno = 0;
    const unsigned long value = strtoul(str_copy,
                                        NULL,
                                        10);
    if (errno != 0) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_OUT_OF_RANGE,
                                           "Failed to convert string to unsigned long."),
                            out_error);
        goto failure;
    }

    free(str_copy);

    *out_value = value;
    return length;

failure:
    if (str_copy) {
        free(str_copy);
    }
    return 0;
}

size_t
cos_scanner_skip_whitespace(CosScanner *scanner)
{
    COS_PARAMETER_ASSERT(scanner != NULL);
    if (!scanner) {
        return 0;
    }

    size_t skipped = 0;

    while (scanner->position < scanner->buffer_size) {
        const char c = scanner->buffer[scanner->position];
        if (!cos_is_whitespace(c)) {
            break;
        }
        else {
            scanner->position++;
            skipped++;
        }
    }

    return skipped;
}

COS_ASSUME_NONNULL_END
