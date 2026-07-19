/*
 * Copyright (c) 2026 OpenCOS.
 */

/*
 * Hybrid-reference file support (/XRefStm).
 *
 * Unlike the other xref fixtures in this directory, this one is assembled at run time rather than
 * hardcoded as a byte array. A hybrid file has three interdependent byte offsets -- startxref to
 * the classic table, the trailer's /XRefStm to the companion stream, and the table's entries to
 * the objects -- so any hand edit to a hardcoded array would silently corrupt it, and this repo
 * has no fixture generator to regenerate one. A single forward pass needs no back-patching,
 * because every offset is known by the time it must be written.
 *
 * The companion xref stream is deliberately uncompressed: /Filter is optional, this repo has an
 * inflater but no deflate compressor, and Flate + Predictor is already covered by xref-stream.c
 * and trailer-chain.c. This file is about entry precedence, not about decoding.
 */

#include "CosTest.h"

#include <libcos/CosDoc.h>
#include <libcos/CosObjID.h>
#include <libcos/CosParser.h>
#include <libcos/CosTrailer.h>
#include <libcos/common/CosError.h>
#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>
#include <libcos/objects/CosIndirectObjNode.h>
#include <libcos/objects/CosObjNode.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

#define HYBRID_BUFFER_SIZE 1024

// The byte offsets chosen while assembling a hybrid fixture.
typedef struct HybridLayout {
    size_t table_offset;    /**< The classic xref table. */
    size_t xref_stm_offset; /**< The companion xref stream object. */
} HybridLayout;

// Appends a NUL-terminated string, advancing @p offset. Returns false if it would overflow.
static bool
append_(unsigned char *buffer,
        size_t capacity,
        size_t *offset,
        const char *text)
{
    const size_t length = strlen(text);
    if (*offset + length > capacity) {
        return false;
    }
    memcpy(buffer + *offset, text, length);
    *offset += length;
    return true;
}

// Appends raw bytes, advancing @p offset. Returns false if it would overflow.
static bool
append_bytes_(unsigned char *buffer,
              size_t capacity,
              size_t *offset,
              const unsigned char *bytes,
              size_t count)
{
    if (*offset + count > capacity) {
        return false;
    }
    memcpy(buffer + *offset, bytes, count);
    *offset += count;
    return true;
}

/*
 * Assembles a hybrid-reference PDF into @p buffer, returning its size (or 0 on overflow).
 *
 * Object 5 is stored in object stream 4. The classic table marks object 5 free -- as a real
 * hybrid file does, so that readers without object-stream support ignore it -- while the /XRefStm
 * describes it as a type 2 entry. Object 1 is in-use in the classic table and absent from the
 * /XRefStm.
 *
 * When @p xref_stm_override is non-negative it replaces the real /XRefStm offset, to exercise the
 * failure path.
 */
static size_t
hybrid_build_pdf_(unsigned char *buffer,
                  size_t capacity,
                  long xref_stm_override,
                  HybridLayout *out_layout)
{
    char scratch[256];
    size_t off = 0;

    if (!append_(buffer, capacity, &off, "%PDF-1.5\n")) {
        return 0;
    }

    // Object 1: an uncompressed object, reached through the classic table.
    const size_t off_catalog = off;
    if (!append_(buffer, capacity, &off,
                 "1 0 obj\n"
                 "<< /Type /Catalog >>\n"
                 "endobj\n")) {
        return 0;
    }

    // Object 4: an object stream holding object 5. Its data is "5 0\n" (the N pairs of object
    // number and relative offset, hence /First 4) followed by object 5 itself.
    const size_t off_objstm = off;
    if (!append_(buffer, capacity, &off,
                 "4 0 obj\n"
                 "<< /Type /ObjStm /N 1 /First 4 /Length 21 >>\n"
                 "stream\n"
                 "5 0\n"
                 "<< /Type /Page >>"
                 "\nendstream\n"
                 "endobj\n")) {
        return 0;
    }

    // Object 3: the companion xref stream. /W [1 1 1] and /Index [5 1] describe exactly one
    // entry -- object 5, type 2, in object stream 4 at index 0 -- so its /Length is 3.
    const size_t off_xref_stm = off;
    if (!append_(buffer, capacity, &off,
                 "3 0 obj\n"
                 "<< /Type /XRef /Size 6 /Index [5 1] /W [1 1 1] /Length 3 >>\n"
                 "stream\n")) {
        return 0;
    }
    static const unsigned char xref_stm_data[] = {0x02, 0x04, 0x00};
    if (!append_bytes_(buffer, capacity, &off, xref_stm_data, sizeof(xref_stm_data))) {
        return 0;
    }
    if (!append_(buffer, capacity, &off,
                 "\nendstream\n"
                 "endobj\n")) {
        return 0;
    }

    // The classic table. Object 5 is free here; only the /XRefStm knows where it really lives.
    const size_t off_table = off;
    snprintf(scratch, sizeof(scratch),
             "xref\n"
             "0 6\n"
             "0000000000 65535 f \n"
             "%010zu 00000 n \n"
             "0000000000 65535 f \n"
             "%010zu 00000 n \n"
             "%010zu 00000 n \n"
             "0000000000 65535 f \n",
             off_catalog, off_xref_stm, off_objstm);
    if (!append_(buffer, capacity, &off, scratch)) {
        return 0;
    }

    const long xref_stm_value = (xref_stm_override >= 0)
                                    ? xref_stm_override
                                    : (long)off_xref_stm;
    snprintf(scratch, sizeof(scratch),
             "trailer\n"
             "<< /Size 6 /Root 1 0 R /XRefStm %ld >>\n"
             "startxref\n"
             "%zu\n"
             "%%%%EOF",
             xref_stm_value, off_table);
    if (!append_(buffer, capacity, &off, scratch)) {
        return 0;
    }

    if (out_layout) {
        out_layout->table_offset = off_table;
        out_layout->xref_stm_offset = off_xref_stm;
    }
    return off;
}

// Builds and parses a hybrid fixture. Returns NULL if the parse fails.
static CosDoc * COS_Nullable
hybrid_parse_(unsigned char *buffer,
              long xref_stm_override,
              HybridLayout *out_layout,
              CosMemoryStream * COS_Nullable * COS_Nullable out_stream)
{
    const size_t size = hybrid_build_pdf_(buffer, HYBRID_BUFFER_SIZE,
                                          xref_stm_override, out_layout);
    if (size == 0) {
        return NULL;
    }

    CosDoc *doc = cos_doc_create(NULL);
    CosMemoryStream *stream = NULL;
    CosParser *parser = NULL;
    CosError error = cos_error_none();

    if (!doc) {
        goto failure;
    }

    stream = cos_memory_stream_create_readonly(buffer, size);
    if (!stream) {
        goto failure;
    }

    parser = cos_parser_create(doc, (CosStream *)stream, NULL);
    if (!parser) {
        goto failure;
    }

    if (!cos_parser_parse(parser, &error)) {
        goto failure;
    }

    if (out_stream) {
        *out_stream = stream;
    }
    return doc;

failure:
    if (doc) {
        cos_doc_destroy(doc);
    }
    if (stream) {
        cos_stream_close((CosStream *)stream);
    }
    return NULL;
}

// The whole point of the feature: object 5 exists only in the /XRefStm, and the classic table
// says it is free. If the classic section took precedence, the free entry would be found first
// and resolution would fail with COS_ERROR_XREF.
static int
compressed_object_resolves_through_xref_stm(void)
{
    unsigned char buffer[HYBRID_BUFFER_SIZE];
    HybridLayout layout = {0, 0};
    CosMemoryStream *stream = NULL;
    CosDoc *doc = hybrid_parse_(buffer, -1, &layout, &stream);
    TEST_EXPECT(doc != NULL);

    CosError error = cos_error_none();
    CosIndirectObjNode *obj = cos_doc_get_object(doc, cos_obj_id_make(5, 0), &error);
    TEST_EXPECT(obj != NULL);
    TEST_EXPECT(cos_obj_node_get_type(cos_indirect_obj_node_get_value(obj)) ==
                CosObjNodeType_Dict);
    cos_obj_node_release((CosObjNode *)obj);

    cos_doc_destroy(doc);
    cos_stream_close((CosStream *)stream);
    return EXIT_SUCCESS;
}

// The /XRefStm section must not shadow the classic table for objects it does not describe.
static int
classic_entries_still_resolve(void)
{
    unsigned char buffer[HYBRID_BUFFER_SIZE];
    HybridLayout layout = {0, 0};
    CosMemoryStream *stream = NULL;
    CosDoc *doc = hybrid_parse_(buffer, -1, &layout, &stream);
    TEST_EXPECT(doc != NULL);

    CosError error = cos_error_none();
    CosIndirectObjNode *catalog = cos_doc_get_object(doc, cos_obj_id_make(1, 0), &error);
    TEST_EXPECT(catalog != NULL);
    TEST_EXPECT(cos_obj_node_get_type(cos_indirect_obj_node_get_value(catalog)) ==
                CosObjNodeType_Dict);
    cos_obj_node_release((CosObjNode *)catalog);

    cos_doc_destroy(doc);
    cos_stream_close((CosStream *)stream);
    return EXIT_SUCCESS;
}

// The companion stream's dictionary is not a trailer: it must not join the revision chain, and
// the sole trailer must be the classic one.
static int
xref_stm_does_not_join_the_trailer_chain(void)
{
    unsigned char buffer[HYBRID_BUFFER_SIZE];
    HybridLayout layout = {0, 0};
    CosMemoryStream *stream = NULL;
    CosDoc *doc = hybrid_parse_(buffer, -1, &layout, &stream);
    TEST_EXPECT(doc != NULL);

    CosTrailer * const trailer = cos_doc_get_trailer(doc);
    TEST_EXPECT(trailer != NULL);
    TEST_EXPECT(cos_trailer_get_prev(trailer) == NULL);
    TEST_EXPECT((size_t)cos_trailer_get_xref_offset(trailer) == layout.table_offset);
    TEST_EXPECT(cos_doc_get_root(doc) != NULL);

    cos_doc_destroy(doc);
    cos_stream_close((CosStream *)stream);
    return EXIT_SUCCESS;
}

// A declared but unparsable /XRefStm is an error rather than a silent fallback to the classic
// table, which would resurface later as a confusing "object is free" failure.
static int
unparsable_xref_stm_fails(void)
{
    unsigned char buffer[HYBRID_BUFFER_SIZE];
    HybridLayout layout = {0, 0};
    CosDoc *doc = hybrid_parse_(buffer, 3, &layout, NULL);
    TEST_EXPECT(doc == NULL);
    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END

TEST_MAIN()
{
    TEST_EXPECT(compressed_object_resolves_through_xref_stm() == EXIT_SUCCESS);
    TEST_EXPECT(classic_entries_still_resolve() == EXIT_SUCCESS);
    TEST_EXPECT(xref_stm_does_not_join_the_trailer_chain() == EXIT_SUCCESS);
    TEST_EXPECT(unparsable_xref_stm_fails() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}
