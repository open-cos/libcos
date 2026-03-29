/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/CosParser.h"

#include "CosDoc-Private.h"
#include "common/Assert.h"
#include "parse/CosBaseParser.h"
#include "parse/CosObjParser.h"

#include <libcos/CosDoc.h>
#include <libcos/common/CosError.h>
#include <libcos/common/memory/CosMemory.h>
#include <libcos/io/CosStream.h>
#include <libcos/objects/CosDictObjNode.h>
#include <libcos/objects/CosObjNode.h>
#include <libcos/syntax/tokenizer/CosTokenizer.h>
#include <libcos/xref/CosXrefTableParser.h>
#include <libcos/xref/table/CosXrefTable.h>

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

struct CosParser {
    CosBaseParser base;       /**< Owns the tokenizer; borrows the stream. */
    CosObjParser *obj_parser; /**< Borrows base.tokenizer. Owned by this parser. */
};

// MARK: - Forward declarations

static bool
cos_parser_parse_header_(CosParser *parser,
                         CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(2);

static bool
cos_parser_find_startxref_(CosParser *parser,
                           CosStreamOffset *out_xref_offset,
                           CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(2)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

static bool
cos_parser_parse_xref_and_trailer_(CosParser *parser,
                                   CosStreamOffset xref_offset,
                                   CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

// MARK: - Public API

CosParser *
cos_parser_create(CosDoc *document,
                  CosStream *input_stream)
{
    COS_API_PARAM_CHECK(document != NULL);
    COS_API_PARAM_CHECK(input_stream != NULL);
    if (!document || !input_stream) {
        return NULL;
    }

    CosParser *parser = NULL;
    CosObjParser *obj_parser = NULL;

    parser = calloc(1, sizeof(CosParser));
    if (!parser) {
        goto failure;
    }

    if (!cos_base_parser_init(&(parser->base), document, input_stream)) {
        goto failure;
    }

    obj_parser = cos_obj_parser_create_with_tokenizer(document,
                                                      parser->base.tokenizer);
    if (!obj_parser) {
        goto failure;
    }

    parser->obj_parser = obj_parser;

    cos_doc_set_parser_(document, parser);

    return parser;

failure:
    if (obj_parser) {
        cos_obj_parser_destroy(obj_parser);
    }
    if (parser) {
        cos_base_parser_destroy(&(parser->base));
    }
    return NULL;
}

void
cos_parser_destroy(CosParser *parser)
{
    if (!parser) {
        return;
    }

    if (parser->obj_parser) {
        cos_obj_parser_destroy(parser->obj_parser);
        parser->obj_parser = NULL;
    }

    cos_base_parser_destroy(&(parser->base));
}

bool
cos_parser_parse(CosParser *parser,
                 CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(parser != NULL);
    if (COS_UNLIKELY(!parser)) {
        return false;
    }

    // Phase 1: Parse the header and extract the PDF version.
    if (!cos_parser_parse_header_(parser, out_error)) {
        return false;
    }

    // Phase 2: Locate the startxref offset near end-of-file.
    CosStreamOffset xref_offset = 0;
    if (!cos_parser_find_startxref_(parser, &xref_offset, out_error)) {
        return false;
    }

    // Phases 3–5: Parse xref table, trailer dict, and root reference.
    if (!cos_parser_parse_xref_and_trailer_(parser, xref_offset, out_error)) {
        return false;
    }

    return true;
}

CosObjNode *
cos_parser_load_object(CosParser *parser,
                       CosStreamOffset byte_offset,
                       CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(parser != NULL);
    if (COS_UNLIKELY(!parser)) {
        return NULL;
    }

    CosStream * const stream = parser->base.input_stream;

    if (!cos_stream_seek(stream, byte_offset, CosStreamOffsetWhence_Set, out_error)) {
        return NULL;
    }

    cos_tokenizer_reset(parser->base.tokenizer);
    cos_obj_parser_flush_tokens_(parser->obj_parser);

    return cos_obj_parser_next_object(parser->obj_parser, out_error);
}

// MARK: - Phase 1: Header parsing

static bool
cos_parser_parse_header_(CosParser *parser,
                         CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);

    CosStream * const stream = parser->base.input_stream;

    if (!cos_stream_seek(stream, 0, CosStreamOffsetWhence_Set, out_error)) {
        return false;
    }

    // "%PDF-X.Y" is 8 bytes minimum; read a bit more to be safe.
    unsigned char header[20];
    memset(header, 0, sizeof(header));

    const size_t bytes_read = cos_stream_read(stream,
                                              header,
                                              sizeof(header) - 1,
                                              out_error);
    if (bytes_read < 8) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_SYNTAX,
                                           "File too short to be a PDF"));
        return false;
    }

    if (memcmp(header, "%PDF-", 5) != 0) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_SYNTAX,
                                           "Not a PDF file: missing %%PDF- header"));
        return false;
    }

    // Header format: "%PDF-M.m" where M is major and m is minor digit.
    const unsigned char major_ch = header[5];
    const unsigned char dot_ch = header[6];
    const unsigned char minor_ch = header[7];

    if (major_ch < '0' || major_ch > '9' ||
        dot_ch != '.' ||
        minor_ch < '0' || minor_ch > '9') {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_SYNTAX,
                                           "Invalid PDF version in header"));
        return false;
    }

    const int version = (int)(major_ch - '0') * 10 + (int)(minor_ch - '0');
    cos_doc_set_version_(parser->base.doc, version);

    return true;
}

// MARK: - Phase 2: startxref backward scan

static bool
cos_parser_find_startxref_(CosParser *parser,
                           CosStreamOffset *out_xref_offset,
                           CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(out_xref_offset != NULL);

    CosStream * const stream = parser->base.input_stream;

    // Seek to end to determine file size.
    if (!cos_stream_seek(stream, 0, CosStreamOffsetWhence_End, out_error)) {
        return false;
    }

    const CosStreamOffset file_size = cos_stream_get_position(stream, out_error);
    if (file_size < 0) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_IO,
                                           "Failed to determine file size"));
        return false;
    }

    // Read up to 1024 bytes from the end of the file.
    const size_t scan_size = ((size_t)file_size < 1024u) ? (size_t)file_size : 1024u;
    const CosStreamOffset scan_start = file_size - (CosStreamOffset)scan_size;

    if (!cos_stream_seek(stream, scan_start, CosStreamOffsetWhence_Set, out_error)) {
        return false;
    }

    CosAllocator * const allocator = parser->base.allocator;
    unsigned char * const buffer = cos_alloc(allocator, scan_size);
    if (!buffer) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to allocate scan buffer"));
        return false;
    }

    const size_t bytes_read = cos_stream_read(stream, buffer, scan_size, out_error);
    if (bytes_read == 0) {
        cos_free(allocator, buffer);
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_IO,
                                           "Failed to read end-of-file region"));
        return false;
    }

    bool result = false;
    CosStreamOffset xref_offset = 0;

    // Scan backwards for "%%EOF".
    int eof_pos = -1;
    for (int i = (int)bytes_read - 5; i >= 0; i--) {
        if (memcmp(&buffer[i], "%%EOF", 5) == 0) {
            eof_pos = i;
            break;
        }
    }

    if (eof_pos < 0) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_SYNTAX,
                                           "%%EOF marker not found"));
        goto cleanup;
    }

    {
        // Scan backwards from %%EOF to find the xref offset integer.
        int pos = eof_pos - 1;

        // Skip whitespace before %%EOF.
        while (pos >= 0 &&
               (buffer[pos] == '\n' || buffer[pos] == '\r' ||
                buffer[pos] == ' ' || buffer[pos] == '\t')) {
            pos--;
        }

        if (pos < 0 || buffer[pos] < '0' || buffer[pos] > '9') {
            cos_error_propagate(out_error,
                                cos_error_make(COS_ERROR_SYNTAX,
                                               "Expected integer before %%EOF"));
            goto cleanup;
        }

        const int int_end = pos;
        while (pos >= 0 && buffer[pos] >= '0' && buffer[pos] <= '9') {
            pos--;
        }
        const int int_start = pos + 1;

        // Parse the integer (iterate forwards through the digits).
        for (int j = int_start; j <= int_end; j++) {
            xref_offset = xref_offset * 10 + (CosStreamOffset)(buffer[j] - '0');
        }

        // Skip whitespace between integer and "startxref".
        while (pos >= 0 &&
               (buffer[pos] == '\n' || buffer[pos] == '\r' ||
                buffer[pos] == ' ' || buffer[pos] == '\t')) {
            pos--;
        }

        // Verify "startxref" keyword (9 characters).
        // pos now points to the 'f' of "startxref"; pos-8 points to 's'.
        if (pos < 8 || memcmp(&buffer[pos - 8], "startxref", 9) != 0) {
            cos_error_propagate(out_error,
                                cos_error_make(COS_ERROR_SYNTAX,
                                               "'startxref' keyword not found"));
            goto cleanup;
        }

        *out_xref_offset = xref_offset;
        result = true;
    }

cleanup:
    cos_free(allocator, buffer);
    return result;
}

// MARK: - Phases 3–5: Xref table, trailer dict, root reference

static bool
cos_parser_parse_xref_and_trailer_(CosParser *parser,
                                   CosStreamOffset xref_offset,
                                   CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);

    CosDoc * const doc = parser->base.doc;
    CosStream * const stream = parser->base.input_stream;

    // Phase 3: Seek to the xref table and parse it.
    if (!cos_stream_seek(stream, xref_offset, CosStreamOffsetWhence_Set, out_error)) {
        return false;
    }

    // Reset the tokenizer so it reads from the new stream position.
    cos_tokenizer_reset(parser->base.tokenizer);
    // Also discard any stale tokens buffered in the obj_parser.
    cos_obj_parser_flush_tokens_(parser->obj_parser);

    CosXrefTableParser * const xtp = cos_xref_table_parser_create(doc,
                                                                  parser->base.tokenizer);
    if (!xtp) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_PARSE,
                                           "Failed to create xref table parser"));
        return false;
    }

    CosXrefTable * const table = cos_xref_table_parser_parse(xtp, out_error);
    cos_xref_table_parser_destroy(xtp);

    if (!table) {
        return false;
    }

    cos_doc_set_xref_table_(doc, table);

    // Phase 4: Parse the trailer dictionary.
    //
    // After cos_xref_table_parser_destroy(), the shared tokenizer is positioned
    // immediately after the "trailer" keyword — the xref table parser peeked it
    // to stop its loop, which caused the tokenizer to consume those bytes from the
    // stream, and the token was released when the parser was destroyed. The next
    // cos_obj_parser_next_object() call will therefore read the trailer dict "<<".
    cos_obj_parser_flush_tokens_(parser->obj_parser);

    CosObjNode * const trailer_obj = cos_obj_parser_next_object(parser->obj_parser, out_error);
    if (!trailer_obj) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_PARSE,
                                           "Failed to parse trailer dictionary"));
        return false;
    }

    if (!cos_obj_node_is_dict(trailer_obj)) {
        cos_obj_node_free(trailer_obj);
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_PARSE,
                                           "Trailer is not a dictionary"));
        return false;
    }

    CosDictObjNode * const trailer_dict = (CosDictObjNode *)trailer_obj;
    cos_doc_set_trailer_dict_(doc, trailer_dict);

    // Phase 5: Extract the /Root reference for lazy object resolution.
    CosObjNode * COS_Nullable root_obj = NULL;
    if (cos_dict_obj_node_get_value_with_string(trailer_dict, "Root", &root_obj, NULL) &&
        root_obj != NULL) {
        // Store as a borrowed reference; trailer_dict owns the object.
        cos_doc_set_root_(doc, root_obj);
    }

    return true;
}

COS_ASSUME_NONNULL_END
