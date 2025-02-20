/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/filters/CosASCIIHexFilter.h"

#include "common/Assert.h"
#include "common/CharacterSet.h"
#include "common/CosError.h"
#include "common/CosMacros.h"

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

enum CosASCIIHexConstants {
    COS_ASCII_HEX_BLOCK_SIZE = 2,
};

struct CosASCIIHexFilterContext {
    /**
     * @brief The buffer used to store decoded or encoded data.
     */
    unsigned char buffer[256];

    /**
     * @brief The number of valid bytes in @c buffer .
     */
    size_t buffer_length;

    /**
     * @brief The index of the next byte to read from @c buffer .
     */
    size_t buffer_index;

    /**
     * @brief Flag to indicate the end of the data stream.
     */
    bool eod;
};

// Private function prototypes

static size_t
cos_ascii_hex_filter_read_(CosStream *stream,
                           void *buffer,
                           size_t count,
                           CosError * COS_Nullable out_error);

static size_t
cos_ascii_hex_filter_write_(CosStream *stream,
                            const void *buffer,
                            size_t count,
                            CosError * COS_Nullable out_error);

static void
cos_ascii_hex_filter_close_(CosStream *stream);

/**
 * @brief Fills the buffer with decoded data.
 *
 * @param ascii_hex_filter The ASCII hexadecimal filter.
 * @param count The number of bytes to read. This number is taken as a hint and may not be met.
 * @param error The error information.
 *
 * @return The number of bytes read.
 */
static size_t
cos_ascii_hex_fill_decode_buffer_(CosASCIIHexFilter *ascii_hex_filter,
                                  size_t count,
                                  CosError * COS_Nullable error);

/**
 * @brief Decodes a pair of ASCII hexadecimal digits.
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

    if (COS_UNLIKELY(!cos_ascii_hex_filter_init(ascii_hex_filter))) {
        goto failure;
    }

    return ascii_hex_filter;

failure:
    if (ascii_hex_filter) {
        free(ascii_hex_filter);
    }
    return NULL;
}

bool
cos_ascii_hex_filter_init(CosASCIIHexFilter *ascii_hex_filter)
{
    COS_API_PARAM_CHECK(ascii_hex_filter != NULL);

    cos_filter_init(&(ascii_hex_filter->base),
                    &(CosStreamFunctions){
                        .read_func = &cos_ascii_hex_filter_read_,
                        .write_func = &cos_ascii_hex_filter_write_,
                        .close_func = &cos_ascii_hex_filter_close_,
                    });

    CosASCIIHexFilterContext * const context = calloc(1, sizeof(CosASCIIHexFilterContext));
    if (COS_UNLIKELY(!context)) {
        return false;
    }

    ascii_hex_filter->context = context;

    return true;
}

// Private function implementations

static size_t
cos_ascii_hex_filter_read_(CosStream *stream,
                           void *buffer,
                           size_t count,
                           CosError * COS_Nullable out_error)
{
    CosASCIIHexFilter * const ascii_hex_filter = (CosASCIIHexFilter *)stream;

    unsigned char * const output_buffer = (unsigned char *)buffer;
    size_t total_read = 0;

    while (total_read < count) {
        // Refill the buffer if necessary.
        if (ascii_hex_filter->context->buffer_index >= ascii_hex_filter->context->buffer_length) {
            if (ascii_hex_filter->context->eod) {
                // No more data to read.
                break;
            }

            const size_t decoded_count = cos_ascii_hex_fill_decode_buffer_(ascii_hex_filter,
                                                                           count - total_read,
                                                                           out_error);
            if (decoded_count == 0) {
                break;
            }

            if (ascii_hex_filter->context->buffer_length == 0) {
                // No more data to read.
                break;
            }
        }

        const size_t read_count = COS_MIN(ascii_hex_filter->context->buffer_length - ascii_hex_filter->context->buffer_index,
                                          count - total_read);
        memcpy(output_buffer + total_read,
               ascii_hex_filter->context->buffer + ascii_hex_filter->context->buffer_index,
               read_count);

        ascii_hex_filter->context->buffer_index += read_count;

        total_read += read_count;
    }

    return total_read;
}

static size_t
cos_ascii_hex_filter_write_(CosStream *stream,
                            const void *buffer,
                            size_t count,
                            CosError * COS_Nullable out_error)
{
    (void)stream;
    (void)buffer;
    (void)count;
    (void)out_error;

    return 0;
}

static void
cos_ascii_hex_filter_close_(CosStream *stream)
{
    COS_IMPL_PARAM_CHECK(stream != NULL);

    CosASCIIHexFilter * const ascii_hex_filter = (CosASCIIHexFilter *)stream;

    free(ascii_hex_filter->context);

    cos_filter_deinit(&(ascii_hex_filter->base));
}

static size_t
cos_ascii_hex_fill_decode_buffer_(CosASCIIHexFilter *ascii_hex_filter,
                                  size_t count,
                                  CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(ascii_hex_filter != NULL);

    // Reset the internal buffer.
    ascii_hex_filter->context->buffer_length = 0;
    ascii_hex_filter->context->buffer_index = 0;

    CosStream * const source_stream = ascii_hex_filter->base.source;
    if (COS_UNLIKELY(!source_stream)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "No source stream"),
                            error);
        return 0;
    }

    const size_t max_read_count = COS_MIN(count, COS_ARRAY_SIZE(ascii_hex_filter->context->buffer));
    size_t total_read = 0;

    while (total_read < max_read_count) {
        unsigned char in_block[COS_ASCII_HEX_BLOCK_SIZE] = {0};
        size_t block_length = 0;
        uint8_t ch;

        while (block_length < COS_ASCII_HEX_BLOCK_SIZE) {
            size_t read_count = cos_stream_read(source_stream, &ch, 1, error);
            if (read_count == 0) {
                // End of underlying stream reached unexpectedly.
                ascii_hex_filter->context->eod = true;
                break;
            }
            if (cos_is_whitespace(ch)) {
                continue;
            }

            // Check for the end marker ">"
            if (ch == '>') {
                ascii_hex_filter->context->eod = true;
                break;
            }

            in_block[block_length++] = ch;
        }

        if (block_length == 0) {
            break;
        }

        if (!cos_ascii_hex_decode_(in_block,
                                   ascii_hex_filter->context->buffer + total_read)) {
            break;
        }
        total_read++;
    }

    ascii_hex_filter->context->buffer_length = total_read;

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
