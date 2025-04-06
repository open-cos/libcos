/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/parse/CosFileParser.h"

#include "common/Assert.h"
#include "common/CharacterSet.h"
#include "common/CosMacros.h"
#include "syntax/tokenizer/CosTokenizer.h"

#include "libcos/io/CosStream.h"
#include "libcos/io/string-support.h"

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

struct CosFileParser {
    /**
     * @brief The document being parsed.
     */
    CosDoc *doc;

    /**
     * @brief The input stream.
     */
    CosStream *input_stream;

    CosTokenizer *tokenizer;

    struct {
        unsigned char data[256];

        /**
         * @brief The number of valid bytes in the buffer.
         */
        size_t count;

        /**
         * The current position in the buffer.
         *
         * @invariant The current position is within the buffer:
         * <code>
         * current >= data && current < data + count
         * </code>
         */
        unsigned char *current;
    } buffer;
};

typedef struct CosFileHeader CosFileHeader;

struct CosFileHeader {
    /**
     * @brief The file header signature.
     */
    unsigned char signature[4];

    /**
     * @brief The major version number of the file.
     */
    unsigned int major_version;

    /**
     * @brief The minor version number of the file.
     */
    unsigned int minor_version;
};

typedef struct CosFileTrailer CosFileTrailer;

struct CosFileTrailer {
    CosDictObj *trailer_dict;

    unsigned int last_xref_offset;

    /**
     * @brief The previous trailer, or @c NULL if this is the first trailer.
     */
    CosFileTrailer * COS_Nullable previous;
};

static bool
cos_parse_file_header_(CosFileParser *parser,
                       CosFileHeader *file_header,
                       CosError * COS_Nullable error);

static bool
cos_parse_file_trailer_(CosFileParser *parser,
                        CosFileTrailer *file_trailer,
                        CosError * COS_Nullable error);

static void
cos_find_last_xref_offset_(CosFileParser *parser,
                           CosFileTrailer *file_trailer,
                           CosError *out_error);

CosFileParser *
cos_file_parser_create(CosDoc *doc,
                       CosStream *input_stream)
{
    COS_API_PARAM_CHECK(doc != NULL);

    CosFileParser *parser = NULL;
    CosTokenizer *tokenizer = NULL;

    parser = calloc(1, sizeof(CosFileParser));
    if (!parser) {
        goto failure;
    }

    tokenizer = cos_tokenizer_create(input_stream);
    if (COS_UNLIKELY(!tokenizer)) {
        goto failure;
    }

    parser->doc = doc;
    parser->input_stream = input_stream;
    parser->tokenizer = tokenizer;

    return parser;

failure:
    if (parser) {
        free(parser);
    }
    if (tokenizer) {
        cos_tokenizer_destroy(tokenizer);
    }
    return NULL;
}

void
cos_file_parser_destroy(CosFileParser *parser)
{
    COS_API_PARAM_CHECK(parser != NULL);
    if (!parser) {
        return;
    }

    free(parser);
}

bool
cos_file_parser_parse(CosFileParser *parser,
                      CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(parser != NULL);
    if (!parser) {
        return false;
    }

    // Parse the file header.
    CosFileHeader file_header = {0};
    if (!cos_parse_file_header_(parser,
                                &file_header,
                                out_error)) {
        goto failure;
    }

    // Parse the file trailer.
    CosFileTrailer file_trailer = {0};
    if (!cos_parse_file_trailer_(parser,
                                 &file_trailer,
                                 out_error)) {
        goto failure;
    }

    // Rewind the input stream.
    if (!cos_stream_seek(parser->input_stream,
                         0,
                         CosStreamOffsetWhence_Set,
                         out_error)) {
        goto failure;
    }

    return true;

failure:
    return false;
}

// MARK: - Private functions

static bool
cos_parse_file_header_(CosFileParser *parser,
                       CosFileHeader *file_header,
                       CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(file_header != NULL);

    // Read the file header from the input stream.
    if (!cos_stream_seek(parser->input_stream,
                         0,
                         CosStreamOffsetWhence_Set,
                         error)) {
        goto failure;
    }

    unsigned char *buffer = parser->buffer.data;

    // Reset the buffer before reading.
    parser->buffer.count = 0;

    size_t read_count = cos_stream_read(parser->input_stream,
                                        buffer,
                                        10,
                                        error);
    if (read_count == 0) {
        goto failure;
    }

    parser->buffer.count = read_count;

    if (read_count < 7) {
        // Invalid file header.
        goto failure;
    }

    if (buffer[0] != CosCharacterSet_PercentSign) {
        // Invalid file header.
        goto failure;
    }

    // Read the signature.
    for (int i = 1; i < 4; i++) {
        const unsigned char ch = buffer[i];
        if (ch == CosCharacterSet_HyphenMinus) {
            // Technically an invalid header.
            break;
        }
        file_header->signature[i - 1] = ch;
    }

    // Skip the hyphen.
    if (buffer[4] != CosCharacterSet_HyphenMinus) {
        // Invalid file header.
        goto failure;
    }

    // Read the major version number.
    if (!cos_is_decimal_digit(buffer[5])) {
        // Invalid file header.
        goto failure;
    }
    else {
        file_header->major_version = buffer[5] - CosCharacterSet_DigitZero;
    }

    // Skip the period.
    if (buffer[6] != CosCharacterSet_FullStop) {
        // Invalid file header.
        goto failure;
    }

    // Read the minor version number.
    if (!cos_is_decimal_digit(buffer[7])) {
        // Invalid file header.
        goto failure;
    }
    else {
        file_header->minor_version = buffer[7] - CosCharacterSet_DigitZero;
    }

    return true;

failure:
    return false;
}

static bool
cos_parse_file_trailer_(CosFileParser *parser,
                        CosFileTrailer *file_trailer,
                        CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(file_trailer != NULL);

    // We want to read the last 256 bytes of the file.
    unsigned char *buffer = parser->buffer.data;
    const size_t buffer_size = sizeof(parser->buffer.data);

    // Seek to before the end of the file.
    if (!cos_stream_seek(parser->input_stream,
                         -(CosStreamOffset)buffer_size,
                         CosStreamOffsetWhence_End,
                         error)) {
        goto failure;
    }

    // Reset the buffer before reading.
    parser->buffer.count = 0;

    const size_t read_count = cos_stream_read(parser->input_stream,
                                              buffer,
                                              buffer_size,
                                              error);
    if (read_count == 0) {
        goto failure;
    }
    else if (read_count < buffer_size) {
        // We didn't read the full buffer.
        goto failure;
    }

    parser->buffer.count = read_count;

    const char *eof_marker = cos_strnrstr((char *)buffer,
                                          read_count,
                                          "%%EOF");
    if (COS_UNLIKELY(!eof_marker)) {
        // Invalid file trailer.
        goto failure;
    }

    parser->buffer.current = buffer + (eof_marker - (char *)buffer);

    cos_find_last_xref_offset_(parser,
                               file_trailer,
                               error);

    return true;

failure:
    return false;
}

static void
cos_find_last_xref_offset_(CosFileParser *parser,
                           CosFileTrailer *file_trailer,
                           CosError *out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(file_trailer != NULL);
    COS_IMPL_PARAM_CHECK(out_error != NULL);
}

COS_ASSUME_NONNULL_END
