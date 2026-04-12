/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/filters/CosASCIIHexFilter.h"

#include "common/Assert.h"
#include "common/CharacterSet.h"
#include "common/CosError.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

enum CosASCIIHexConstants {
    COS_ASCII_HEX_BLOCK_SIZE = 2,
};

// Private function prototypes

static bool
cos_ascii_hex_filter_init_(CosASCIIHexFilter *ascii_hex_filter);

static size_t
cos_ascii_hex_fill_decode_buffer_(CosFilter *filter,
                                   CosError * COS_Nullable out_error);

/**
 * Decodes a pair of ASCII hexadecimal digits.
 *
 * @param input The ASCII hexadecimal digit pair.
 * @param output The output buffer for the decoded byte value.
 *
 * @return @c true if the input was successfully decoded, @c false otherwise.
 */
static bool
cos_ascii_hex_decode_(const unsigned char input[2],
                      unsigned char *output)
    COS_ATTR_ACCESS_READ_ONLY(1)
    COS_ATTR_ACCESS_WRITE_ONLY(2);

COS_ATTR_PURE
COS_STATIC_INLINE int
cos_hex_digit_value(unsigned char character);

// Public function implementations

CosASCIIHexFilter *
cos_ascii_hex_filter_create(void)
{
    CosASCIIHexFilter * const ascii_hex_filter = calloc(1, sizeof(CosASCIIHexFilter));
    if (COS_UNLIKELY(!ascii_hex_filter)) {
        goto failure;
    }

    if (COS_UNLIKELY(!cos_ascii_hex_filter_init_(ascii_hex_filter))) {
        goto failure;
    }

    return ascii_hex_filter;

failure:
    if (ascii_hex_filter) {
        free(ascii_hex_filter);
    }
    return NULL;
}

static bool
cos_ascii_hex_filter_init_(CosASCIIHexFilter *ascii_hex_filter)
{
    COS_IMPL_PARAM_CHECK(ascii_hex_filter != NULL);

    static const CosFilterFunctions ascii_hex_filter_functions_ = {
        .decode_func = &cos_ascii_hex_fill_decode_buffer_,
        .encode_func = NULL,
        .close_func  = NULL,
    };

    cos_filter_init(&(ascii_hex_filter->base),
                    &ascii_hex_filter_functions_);

    return true;
}

// Private function implementations

static size_t
cos_ascii_hex_fill_decode_buffer_(CosFilter *filter,
                                   CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(filter != NULL);

    CosStream * const source_stream = filter->source;
    if (COS_UNLIKELY(!source_stream)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "No source stream"),
                            error);
        return 0;
    }

    size_t total_read = 0;

    while (total_read < COS_FILTER_BUFFER_SIZE) {
        unsigned char in_block[COS_ASCII_HEX_BLOCK_SIZE] = {0};
        size_t block_length = 0;
        uint8_t ch;

        while (block_length < COS_ASCII_HEX_BLOCK_SIZE) {
            size_t read_count = cos_stream_read(source_stream, &ch, 1, error);
            if (read_count == 0) {
                // End of underlying stream reached unexpectedly.
                filter->buffer.eod = true;
                break;
            }
            if (cos_is_whitespace(ch)) {
                continue;
            }

            // Check for the end marker ">"
            if (ch == '>') {
                filter->buffer.eod = true;
                break;
            }

            in_block[block_length++] = ch;
        }

        if (block_length == 0) {
            break;
        }

        /* PDF spec section 7.3.4.2: a trailing odd digit A means A0. */
        if (block_length == 1) {
            in_block[1] = '0';
        }

        if (!cos_ascii_hex_decode_(in_block,
                                   filter->buffer.data + total_read)) {
            break;
        }
        total_read++;
    }

    filter->buffer.length = total_read;

    return total_read;
}

static bool
cos_ascii_hex_decode_(const unsigned char input[2],
                      unsigned char *output)
{
    COS_IMPL_PARAM_CHECK(input != NULL);
    COS_IMPL_PARAM_CHECK(output != NULL);

    // Decode the ASCII hexadecimal digit pair.
    uint8_t value = 0;

    for (size_t i = 0; i < 2; i++) {
        const unsigned char character = input[i];
        const int digit_value = cos_hex_digit_value(character);
        if (COS_UNLIKELY(digit_value < 0)) {
            // Invalid character
            return false;
        }
        value = (uint8_t)(value << 4) | (digit_value & 0xF);
    }

    *output = value;

    return true;
}

COS_STATIC_INLINE int
cos_hex_digit_value(unsigned char character)
{
    if (character >= CosCharacterSet_DigitZero &&
        character <= CosCharacterSet_DigitNine) {
        return (character - CosCharacterSet_DigitZero);
    }
    else if (character >= CosCharacterSet_LatinCapitalLetterA &&
             character <= CosCharacterSet_LatinCapitalLetterF) {
        return (character - CosCharacterSet_LatinCapitalLetterA) + 0xA;
    }
    else if (character >= CosCharacterSet_LatinSmallLetterA &&
             character <= CosCharacterSet_LatinSmallLetterF) {
        return (character - CosCharacterSet_LatinSmallLetterA) + 0xA;
    }

    return -1;
}

COS_ASSUME_NONNULL_END
