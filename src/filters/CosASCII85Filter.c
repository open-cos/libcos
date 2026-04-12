/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/filters/CosASCII85Filter.h"

#include "common/Assert.h"
#include "common/CharacterSet.h"
#include "common/CosError.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

enum CosASCII85Constants {
    /**
     * @brief The base character for ASCII base-85 encoding (ASCII 33 = '!').
     */
    COS_ASCII85_BASE = '!',

    /**
     * @brief Special character representing four zero bytes (ASCII 122 = 'z').
     */
    COS_ASCII85_ZERO_CHAR = 'z',

    /**
     * @brief Character used for padding (ASCII 117 = 'u').
     */
    COS_ASCII85_PAD_CHAR = 'u',

    /**
     * @brief The number of characters in a full ASCII base-85 block.
     */
    COS_ASCII85_BLOCK_SIZE = 5,

    /**
     * @brief The number of binary bytes encoded in a full ASCII base-85 block.
     */
    COS_ASCII85_BYTES_PER_BLOCK = 4,

    /**
     * @brief Base of the encoding (number of possible values per character).
     */
    COS_ASCII85_RADIX = 85,
};

// Private function prototypes

static bool
cos_ascii85_filter_init_(CosASCII85Filter *ascii_85_filter);

static size_t
cos_ascii85_fill_decode_buffer_(CosFilter *filter,
                                CosError * COS_Nullable error);

/**
 * Decodes a block of ASCII base-85 encoded data.
 *
 * @param input The ASCII base-85 encoded input data.
 * @param input_count The number of bytes in the input data.
 * @param out The output buffer for the decoded data.
 *
 * @return The number of bytes written to the output buffer.
 */
static size_t
cos_decode_ascii85_block_(const unsigned char *input,
                          size_t input_count,
                          unsigned char *out)
    COS_ATTR_ACCESS_READ_ONLY_SIZE(1, 2)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

// Public functions

CosASCII85Filter *
cos_ascii85_filter_create(void)
{
    CosASCII85Filter * const ascii_85_filter = calloc(1, sizeof(CosASCII85Filter));
    if (COS_UNLIKELY(!ascii_85_filter)) {
        goto failure;
    }

    if (COS_UNLIKELY(!cos_ascii85_filter_init_(ascii_85_filter))) {
        goto failure;
    }

    return ascii_85_filter;

failure:
    if (ascii_85_filter) {
        free(ascii_85_filter);
    }
    return NULL;
}

static bool
cos_ascii85_filter_init_(CosASCII85Filter *ascii_85_filter)
{
    COS_IMPL_PARAM_CHECK(ascii_85_filter != NULL);

    static const CosFilterFunctions ascii85_filter_functions_ = {
        .decode_func = &cos_ascii85_fill_decode_buffer_,
        .encode_func = NULL,
        .close_func  = NULL,
    };

    cos_filter_init(&(ascii_85_filter->base),
                    &ascii85_filter_functions_);

    return true;
}

// Function implementations

static size_t
cos_ascii85_fill_decode_buffer_(CosFilter *filter,
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

    unsigned char in_block[COS_ASCII85_BLOCK_SIZE];
    size_t count = 0;
    uint8_t ch;

    // Read up to 5 non-whitespace characters from the source.
    while (count < COS_ASCII85_BLOCK_SIZE) {
        size_t read_count = cos_stream_read(source_stream, &ch, 1, error);
        if (read_count == 0) {
            // End of underlying stream reached unexpectedly.
            filter->buffer.eod = true;
            break;
        }
        if (cos_is_whitespace(ch)) {
            continue;
        }

        // Check for the end marker "~>"
        if (ch == '~') {
            uint8_t next_ch;
            read_count = cos_stream_read(source_stream, &next_ch, 1, error);
            if (read_count == 0 || next_ch != '>') {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                                   "Malformed end-of-data marker"),
                                    error);
            }
            filter->buffer.eod = true;
            break;
        }

        // Handle the special 'z' case which represents 4 zero bytes
        if (ch == COS_ASCII85_ZERO_CHAR && count == 0) {
            // Directly output four zero bytes
            filter->buffer.data[0] = 0;
            filter->buffer.data[1] = 0;
            filter->buffer.data[2] = 0;
            filter->buffer.data[3] = 0;
            filter->buffer.length = 4;
            return 4;
        }

        // Otherwise, store the character
        in_block[count++] = ch;
    }

    if (count == 0) {
        return 0;
    }

    const size_t decoded_bytes = cos_decode_ascii85_block_(in_block,
                                                           count,
                                                           filter->buffer.data);
    filter->buffer.length = decoded_bytes;
    return decoded_bytes;
}

static size_t
cos_decode_ascii85_block_(const unsigned char *input,
                          size_t input_count,
                          unsigned char *out)
{
    COS_IMPL_PARAM_CHECK(input != NULL);
    COS_IMPL_PARAM_CHECK(out != NULL);

    // The input ASCII base-85 block must contain 1 to 5 characters.
    if (input_count < 1 || input_count > COS_ASCII85_BLOCK_SIZE) {
        return 0;
    }

    // Decode the ASCII base-85 block.
    uint32_t value = 0;

    // Process the provided characters.
    // Each character is in the range 33 ('!') to 117 ('u').
    // We subtract the base (33) to get a digit (0..84).
    for (size_t i = 0; i < input_count; i++) {
        const unsigned char character = input[i];
        if (COS_UNLIKELY(character < COS_ASCII85_BASE ||
                         character > (COS_ASCII85_BASE + COS_ASCII85_RADIX - 1))) {
            // Invalid character
            return 0;
        }
        value = (value * COS_ASCII85_RADIX) + (character - COS_ASCII85_BASE);
    }

    // Handle the special case of a block with fewer than 5 characters.
    if (input_count < COS_ASCII85_BLOCK_SIZE) {
        // For a partial (final) block, pad with the digit corresponding to 'u' (i.e., 84)
        for (size_t i = input_count; i < COS_ASCII85_BLOCK_SIZE; i++) {
            value = (value * COS_ASCII85_RADIX) + (COS_ASCII85_PAD_CHAR - COS_ASCII85_BASE);
        }
    }

    // A block produces input_count-1 bytes.
    const size_t output_byte_count = input_count - 1;
    COS_ALWAYS(output_byte_count <= 4);

    // Write the 32-bit value to output in big-endian order.
    for (unsigned int i = 0; i < output_byte_count; i++) {
        // Extract each byte using bit shifting and masking.
        const unsigned int shift = 8 * (COS_ASCII85_BYTES_PER_BLOCK - 1 - i);
        out[i] = (value >> shift) & UCHAR_MAX;
    }

    return output_byte_count;
}

COS_ASSUME_NONNULL_END
