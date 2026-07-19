/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/CosParser.h"

#include "CosDoc-Private.h"
#include "CosTrailer-Private.h"
#include "common/Assert.h"
#include "objects/CosStreamObjNode-Private.h"
#include "parse/CosBaseParser.h"
#include "parse/CosObjParser.h"
#include "xref/CosXrefStreamParser.h"

#include <libcos/CosDoc.h>
#include <libcos/CosTrailer.h>
#include <libcos/common/CosArray.h>
#include <libcos/common/CosError.h>
#include <libcos/common/memory/CosMemory.h>
#include <libcos/io/CosStream.h>
#include <libcos/objects/CosDictObjNode.h>
#include <libcos/objects/CosIndirectObjNode.h>
#include <libcos/objects/CosObjNode.h>
#include <libcos/objects/CosStreamObjNode.h>
#include <libcos/syntax/tokenizer/CosTokenizer.h>
#include <libcos/objects/CosIntObjNode.h>
#include <libcos/xref/CosXrefTableParser.h>
#include <libcos/xref/table/CosXrefSection.h>
#include <libcos/xref/table/CosXrefTable.h>

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

struct CosParser {
    CosBaseParser base;       /**< Owns the tokenizer; borrows the stream. */
    CosObjParser *obj_parser; /**< Borrows base.tokenizer. Owned by this parser. */
};

/**
 * The maximum number of revisions traversed when walking a cross-reference /Prev chain.
 *
 * This bounds chains that are long but acyclic, which the visited-offset set cannot detect: a
 * malformed file can name thousands of distinct plausible offsets. It also bounds the visited
 * set's linear membership scan.
 */
#define COS_XREF_MAX_REVISIONS 1024

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

static bool
cos_parser_xref_is_stream_(CosParser *parser,
                           CosStreamOffset offset,
                           bool *out_is_stream,
                           CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(3)
    COS_ATTR_ACCESS_WRITE_ONLY(4);

static CosXrefSection * COS_Nullable
cos_parser_parse_xref_stream_at_(CosParser *parser,
                                 CosStreamOffset offset,
                                 CosDictObjNode * COS_Nullable * COS_Nullable out_dict,
                                 CosError * COS_Nullable out_error)
    COS_OWNERSHIP_RETURNS
    COS_ATTR_ACCESS_WRITE_ONLY(3)
    COS_ATTR_ACCESS_WRITE_ONLY(4);

static bool
cos_parser_offsets_contains_(const CosArray *offsets,
                             CosStreamOffset offset);

static bool
cos_parser_offsets_add_(CosArray *offsets,
                        CosStreamOffset offset,
                        CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

// MARK: - Public API

CosParser *
cos_parser_create(CosDoc *document,
                  CosStream *input_stream,
                  const CosParserOptions * COS_Nullable options)
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

    if (!cos_base_parser_init(&(parser->base), document, input_stream, options)) {
        goto failure;
    }

    // The same options must reach the object parser: it shares this tokenizer,
    // and objects loaded through it are judged by the same rules.
    obj_parser = cos_obj_parser_create_with_tokenizer(document,
                                                      parser->base.tokenizer,
                                                      options);
    if (!obj_parser) {
        goto failure;
    }

    parser->obj_parser = obj_parser;

    // Recorded on the document so that consumers without a parser in scope,
    // such as reference resolution, see the same options.
    cos_doc_set_parser_options_(document, options);

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

    // An xref entry points at an indirect object definition.
    cos_obj_parser_set_top_level_flags_(parser->obj_parser,
                                        CosObjParserFlag_IndirectObjDef);

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
        //
        // The digit run is unbounded, so the accumulation is checked against
        // the file size rather than left to overflow: an offset past the end of
        // the file cannot name an xref section, and overflowing CosStreamOffset
        // is undefined behaviour that traps under -ftrapv and silently wraps to
        // a plausible-looking offset otherwise.
        for (int j = int_start; j <= int_end; j++) {
            const CosStreamOffset digit = (CosStreamOffset)(buffer[j] - '0');

            if (xref_offset > ((file_size - digit) / 10)) {
                cos_error_propagate(out_error,
                                    cos_error_make(COS_ERROR_SYNTAX,
                                                   "startxref offset out of range"));
                goto cleanup;
            }

            xref_offset = (xref_offset * 10) + digit;
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

    // Create the master xref table that will accumulate sections from all revisions.
    CosXrefTable *table = cos_xref_table_create();
    if (!table) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to create xref table"));
        return false;
    }

    bool result = false;

    // The trailer revision chain, newest (head) to oldest. `tail` is the oldest linked so far.
    CosTrailer *head = NULL;
    CosTrailer *tail = NULL;

    // The xref offsets already visited, to break /Prev cycles. A revisited /Prev offset is a hard
    // error, while a revisited /XRefStm is merely skipped, so the two are tracked separately: one
    // shared set would spuriously reject a file that reaches the same xref stream both as a
    // revision's /XRefStm and as another revision's /Prev target.
    CosArray *visited = cos_array_create(sizeof(CosStreamOffset), NULL, 8);
    CosArray *visited_xref_stms = cos_array_create(sizeof(CosStreamOffset), NULL, 2);
    size_t revision_count = 0;

    if (!visited || !visited_xref_stms) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to create xref offset set"));
        goto done;
    }

    // Walk the Prev chain from newest to oldest, accumulating xref sections and trailers.
    while (true) {
        if (cos_parser_offsets_contains_(visited, xref_offset)) {
            cos_error_propagate(out_error,
                                cos_error_make(COS_ERROR_XREF,
                                               "Cross-reference /Prev chain contains a cycle"));
            goto done;
        }
        if (!cos_parser_offsets_add_(visited, xref_offset, out_error)) {
            goto done;
        }
        if (++revision_count > COS_XREF_MAX_REVISIONS) {
            cos_error_propagate(out_error,
                                cos_error_make(COS_ERROR_XREF,
                                               "Cross-reference /Prev chain is too long"));
            goto done;
        }

        // Determine whether this revision uses a classic xref table or an xref stream.
        bool is_stream = false;
        if (!cos_parser_xref_is_stream_(parser, xref_offset, &is_stream, out_error)) {
            goto done;
        }

        CosXrefSection *section = NULL;
        // The companion xref stream of a hybrid-reference file, if this revision declares one.
        CosXrefSection *xref_stm_section = NULL;
        // The trailer dictionary for this revision, owned here until handed to a CosTrailer.
        CosDictObjNode *trailer_dict = NULL;

        if (!is_stream) {
            // Seek to the xref section and reset the tokenizer for the parse.
            if (!cos_stream_seek(stream, xref_offset, CosStreamOffsetWhence_Set, out_error)) {
                goto done;
            }
            cos_tokenizer_reset(parser->base.tokenizer);
            cos_obj_parser_flush_tokens_(parser->obj_parser);

            CosXrefTableParser * const xtp =
                cos_xref_table_parser_create(doc,
                                             parser->base.tokenizer,
                                             &(parser->base.options));
            if (!xtp) {
                cos_error_propagate(out_error,
                                    cos_error_make(COS_ERROR_PARSE,
                                                   "Failed to create xref table parser"));
                goto done;
            }

            section = cos_xref_table_parser_parse_section(xtp, out_error);
            cos_xref_table_parser_destroy(xtp);
            if (!section) {
                goto done;
            }

            // After cos_xref_table_parser_destroy(), the shared tokenizer is positioned
            // immediately after the "trailer" keyword, so the next object is the trailer dict.
            cos_obj_parser_flush_tokens_(parser->obj_parser);

            // The trailer is a bare dictionary.
            cos_obj_parser_set_top_level_flags_(parser->obj_parser,
                                                CosObjParserFlag_DictObj);

            CosObjNode * const trailer_obj =
                cos_obj_parser_next_object(parser->obj_parser, out_error);
            if (!trailer_obj) {
                cos_xref_section_destroy(section);
                cos_error_propagate(out_error,
                                    cos_error_make(COS_ERROR_PARSE,
                                                   "Failed to parse trailer dictionary"));
                goto done;
            }
            if (!cos_obj_node_is_dict(trailer_obj)) {
                cos_obj_node_release(trailer_obj);
                cos_xref_section_destroy(section);
                cos_error_propagate(out_error,
                                    cos_error_make(COS_ERROR_PARSE,
                                                   "Trailer is not a dictionary"));
                goto done;
            }
            trailer_dict = (CosDictObjNode *)trailer_obj;

            // A hybrid-reference file's classic trailer names a companion xref stream holding the
            // entries for objects stored in object streams -- the very objects this classic table
            // deliberately marks free so that readers without object-stream support ignore them.
            // Its entries must therefore win over this revision's classic entries.
            CosObjNode * COS_Nullable xref_stm_obj = NULL;
            if (cos_dict_obj_node_get_value_with_string(trailer_dict, "XRefStm",
                                                        &xref_stm_obj, NULL) &&
                xref_stm_obj != NULL &&
                cos_obj_node_is_integer(COS_nonnull_cast(xref_stm_obj))) {
                const int xref_stm_value =
                    cos_int_obj_node_get_value((CosIntObjNode *)xref_stm_obj);
                const CosStreamOffset xref_stm_offset = (CosStreamOffset)xref_stm_value;

                if (xref_stm_value >= 0 &&
                    !cos_parser_offsets_contains_(visited_xref_stms, xref_stm_offset)) {
                    if (!cos_parser_offsets_add_(visited_xref_stms, xref_stm_offset, out_error)) {
                        cos_xref_section_destroy(section);
                        cos_dict_obj_node_destroy(trailer_dict);
                        goto done;
                    }

                    // The companion stream's dictionary is not a trailer: it must not join the
                    // revision chain, contribute /Root, or have its own /Prev followed -- the
                    // classic trailer's /Prev already covers this revision's predecessor.
                    xref_stm_section = cos_parser_parse_xref_stream_at_(parser,
                                                                        xref_stm_offset,
                                                                        NULL,
                                                                        out_error);
                    if (!xref_stm_section) {
                        cos_xref_section_destroy(section);
                        cos_dict_obj_node_destroy(trailer_dict);
                        goto done;
                    }
                }
            }
        }
        else {
            // An xref stream is an indirect object whose value is a stream; its dictionary
            // carries both the xref parameters and the trailer entries, so it is taken here and
            // reused as this revision's trailer.
            section = cos_parser_parse_xref_stream_at_(parser,
                                                       xref_offset,
                                                       &trailer_dict,
                                                       out_error);
            if (!section) {
                goto done;
            }
        }

        // Insertion order fixes precedence: cos_xref_table_find_entry_for_obj_num returns the
        // first section whose subsection range covers the object number, so a revision's /XRefStm
        // section must precede its classic section, and both must precede the older revisions
        // appended on later iterations.
        //
        // A non-conforming /XRefStm that omits /Index (defaulting to [0, /Size)) contributes free
        // entries for objects it does not own, which then shadow the classic table. Skipping free
        // entries during lookup would not fix this and would break legitimate deletions: an object
        // deleted in revision N is free in N and in-use in N-1, and the free entry must win.
        if (xref_stm_section) {
            if (!cos_xref_table_add_section(table, xref_stm_section, out_error)) {
                cos_xref_section_destroy(xref_stm_section);
                cos_xref_section_destroy(section);
                cos_dict_obj_node_destroy(trailer_dict);
                goto done;
            }
            xref_stm_section = NULL; // Ownership transferred to the table.
        }

        if (!cos_xref_table_add_section(table, section, out_error)) {
            // Any /XRefStm section is already owned by the table, so it must not be destroyed here.
            cos_xref_section_destroy(section);
            cos_dict_obj_node_destroy(trailer_dict);
            goto done;
        }

        // Build a trailer node for this revision and link it into the chain.
        CosTrailer * const trailer = cos_trailer_create(trailer_dict, xref_offset);
        if (!trailer) {
            cos_dict_obj_node_destroy(trailer_dict);
            cos_error_propagate(out_error,
                                cos_error_make(COS_ERROR_MEMORY,
                                               "Failed to create trailer"));
            goto done;
        }
        trailer_dict = NULL; // Ownership transferred to the trailer.

        if (!head) {
            head = trailer;
            tail = trailer;

            // Encryption is not supported, so reject the document up front rather than letting the
            // filter chain fail later on undecryptable stream data. /Encrypt is normally an
            // indirect reference, so only its presence is checked.
            CosObjNode * COS_Nullable encrypt_obj = NULL;
            if (cos_dict_obj_node_get_value_with_string(cos_trailer_get_dict(trailer), "Encrypt",
                                                        &encrypt_obj, NULL) &&
                encrypt_obj != NULL) {
                cos_error_propagate(out_error,
                                    cos_error_make(COS_ERROR_NOT_IMPLEMENTED,
                                                   "Encrypted documents are not supported"));
                goto done;
            }

            // Resolve /Root from the newest trailer.
            CosObjNode * COS_Nullable root_obj = NULL;
            if (cos_dict_obj_node_get_value_with_string(cos_trailer_get_dict(trailer), "Root",
                                                        &root_obj, NULL) &&
                root_obj != NULL) {
                cos_doc_set_root_(doc, root_obj);
            }
        }
        else {
            cos_trailer_set_prev_(tail, trailer);
            tail = trailer;
        }

        // Check /Prev to decide whether to continue traversing older revisions.
        CosStreamOffset prev_offset = -1;
        CosObjNode * COS_Nullable prev_obj = NULL;
        if (cos_dict_obj_node_get_value_with_string(cos_trailer_get_dict(trailer), "Prev",
                                                    &prev_obj, NULL) &&
            prev_obj != NULL &&
            cos_obj_node_is_integer(COS_nonnull_cast(prev_obj))) {
            const int prev_value = cos_int_obj_node_get_value((CosIntObjNode *)prev_obj);
            if (prev_value >= 0) {
                prev_offset = (CosStreamOffset)prev_value;
            }
        }

        if (prev_offset < 0) {
            break;
        }
        xref_offset = prev_offset;
    }

    cos_doc_set_trailer_(doc, head); // transfers ownership of the whole chain
    head = NULL;
    cos_doc_set_xref_table_(doc, table);
    table = NULL; // ownership transferred
    result = true;

done:
    if (visited) {
        cos_array_destroy(visited);
    }
    if (visited_xref_stms) {
        cos_array_destroy(visited_xref_stms);
    }
    if (head) {
        cos_trailer_destroy(head);
    }
    if (table) {
        cos_xref_table_destroy(table);
    }
    return result;
}

// Parses the cross-reference stream at @p offset into a section. When @p out_dict is non-NULL, the
// stream's dictionary is detached and returned through it, transferring ownership to the caller;
// otherwise the dictionary is released along with the stream object. Used both for stream-headed
// revisions, which reuse the dictionary as the revision's trailer, and for the /XRefStm companion
// stream of a hybrid-reference file, whose dictionary is not a trailer.
static CosXrefSection * COS_Nullable
cos_parser_parse_xref_stream_at_(CosParser *parser,
                                 CosStreamOffset offset,
                                 CosDictObjNode * COS_Nullable * COS_Nullable out_dict,
                                 CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);

    if (!cos_stream_seek(parser->base.input_stream, offset,
                         CosStreamOffsetWhence_Set, out_error)) {
        return NULL;
    }
    cos_tokenizer_reset(parser->base.tokenizer);
    cos_obj_parser_flush_tokens_(parser->obj_parser);

    // An xref stream is an indirect object definition.
    cos_obj_parser_set_top_level_flags_(parser->obj_parser,
                                        CosObjParserFlag_IndirectObjDef);

    CosObjNode * const obj = cos_obj_parser_next_object(parser->obj_parser, out_error);
    if (!obj) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_PARSE,
                                           "Failed to parse xref stream object"));
        return NULL;
    }
    if (!cos_obj_node_is_indirect(obj)) {
        cos_obj_node_release(obj);
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_XREF,
                                           "Xref stream is not an indirect object"));
        return NULL;
    }

    CosObjNode * const value = cos_indirect_obj_node_get_value((CosIndirectObjNode *)obj);
    if (!value || !cos_obj_node_is_stream(value)) {
        cos_obj_node_release(obj);
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_XREF,
                                           "Xref stream object is not a stream"));
        return NULL;
    }

    CosStreamObjNode * const stream_obj = (CosStreamObjNode *)value;
    CosXrefSection * const section = cos_xref_stream_parse_section_(stream_obj, out_error);
    if (!section) {
        cos_obj_node_release(obj);
        return NULL;
    }

    if (out_dict) {
        // Detach the dictionary so that releasing the indirect object (and with it the stream
        // node) does not destroy it.
        CosDictObjNode * const dict = cos_stream_obj_node_take_dict_(stream_obj);
        cos_obj_node_release(obj);
        if (!dict) {
            cos_xref_section_destroy(section);
            cos_error_propagate(out_error,
                                cos_error_make(COS_ERROR_XREF,
                                               "Xref stream has no dictionary"));
            return NULL;
        }
        *out_dict = dict;
    }
    else {
        cos_obj_node_release(obj);
    }

    return section;
}

static bool
cos_parser_offsets_contains_(const CosArray *offsets,
                             CosStreamOffset offset)
{
    COS_IMPL_PARAM_CHECK(offsets != NULL);

    const size_t count = cos_array_get_count(offsets);
    for (size_t i = 0; i < count; i++) {
        CosStreamOffset item = 0;
        if (!cos_array_get_item(offsets, i, &item, NULL)) {
            continue;
        }
        if (item == offset) {
            return true;
        }
    }
    return false;
}

static bool
cos_parser_offsets_add_(CosArray *offsets,
                        CosStreamOffset offset,
                        CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(offsets != NULL);

    return cos_array_append_item(offsets, &offset, out_error);
}

// Determines whether the cross-reference data at @p offset is a cross-reference stream (an
// indirect object) rather than a classic `xref` keyword table. The stream position is left
// unspecified; the caller re-seeks before parsing.
static bool
cos_parser_xref_is_stream_(CosParser *parser,
                           CosStreamOffset offset,
                           bool *out_is_stream,
                           CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(out_is_stream != NULL);

    CosStream * const stream = parser->base.input_stream;
    if (!cos_stream_seek(stream, offset, CosStreamOffsetWhence_Set, out_error)) {
        return false;
    }

    unsigned char buffer[16];
    const size_t read_count = cos_stream_read(stream, buffer, sizeof(buffer), out_error);
    if (read_count == 0) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_SYNTAX,
                                           "Failed to read cross-reference data"));
        return false;
    }

    // Skip leading whitespace and end-of-line markers.
    size_t i = 0;
    while (i < read_count &&
           (buffer[i] == ' ' || buffer[i] == '\t' ||
            buffer[i] == '\r' || buffer[i] == '\n' || buffer[i] == '\f')) {
        i++;
    }

    // A cross-reference stream is an indirect object, so it begins with an object number; a
    // classic table begins with the 'xref' keyword. Anything else is left to the classic parser
    // to reject.
    *out_is_stream = (i < read_count && buffer[i] >= '0' && buffer[i] <= '9');
    return true;
}

COS_ASSUME_NONNULL_END
