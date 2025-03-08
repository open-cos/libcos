/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/parse/CosFileParser.h"

#include "common/Assert.h"
#include "common/CharacterSet.h"
#include "common/CosMacros.h"

#include "libcos/io/CosStream.h"

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
};

static bool
cos_parse_file_header_(CosFileParser *parser,
                       CosFileHeader *file_header,
                       CosError * COS_Nullable error);

static bool
cos_parse_file_trailer_(CosFileParser *parser,
                        CosFileTrailer *file_trailer,
                        CosError * COS_Nullable error);

CosFileParser *
cos_file_parser_create(CosDoc *doc,
                       CosStream *input_stream)
{
    COS_API_PARAM_CHECK(doc != NULL);

    CosFileParser * const parser = calloc(1, sizeof(CosFileParser));
    if (!parser) {
        return NULL;
    }

    parser->doc = doc;
    parser->input_stream = input_stream;

    return parser;
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

    unsigned char buffer[10] = {0};

    size_t read_count = cos_stream_read(parser->input_stream,
                                        buffer,
                                        sizeof(buffer) - 1,
                                        error);
    if (read_count == 0) {
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
cos_parse_file_trailer(CosFileParser *parser,
                       CosFileTrailer *file_trailer,
                       CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(file_trailer != NULL);

    // We want to read the last 256 bytes of the file.
    unsigned char buffer[256] = {0};

    // Seek to before the end of the file.
    if (!cos_stream_seek(parser->input_stream,
                         -(CosStreamOffset)sizeof(buffer),
                         CosStreamOffsetWhence_End,
                         error)) {
        goto failure;
    }

    const size_t read_count = cos_stream_read(parser->input_stream,
                                              buffer,
                                              sizeof(buffer),
                                              error);
    if (read_count == 0) {
        goto failure;
    }

    // Read backwards from the end of the buffer, searching for the "%%EOF" marker.
    // We can simplify this by searching for the comment start (%) and then checking
    // the next three characters.
    for (size_t i = sizeof(buffer) - 1; i > 0; i--) {
        if (buffer[i] == CosCharacterSet_PercentSign) {
            if (buffer[i + 1] == CosCharacterSet_PercentSign &&
                buffer[i + 2] == CosCharacterSet_LatinCapitalLetterE &&
                buffer[i + 3] == CosCharacterSet_LatinCapitalLetterO &&
                buffer[i + 4] == CosCharacterSet_LatinCapitalLetterF) {
                // Found the "%%EOF" marker.
                break;
            }
        }
    }

failure:
    return false;
}

COS_ASSUME_NONNULL_END
