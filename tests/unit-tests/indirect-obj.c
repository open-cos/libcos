/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "CosTest.h"

#include <libcos/CosDoc.h>
#include <libcos/CosObjID.h>
#include <libcos/CosParser.h>
#include <libcos/common/CosError.h>
#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>
#include <libcos/objects/CosIndirectObjNode.h>
#include <libcos/objects/CosIntObjNode.h>
#include <libcos/objects/CosObjNode.h>

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

// MARK: - Helpers

/**
 * @brief Parses an in-memory PDF and returns the document.
 *
 * On success the caller owns the returned @c CosDoc (free with
 * @c cos_doc_destroy) and the output stream (close with
 * @c cos_stream_close).  Both must be kept alive together because the
 * parser borrows the stream.
 *
 * @param input         NUL-terminated PDF bytes.
 * @param out_stream    On success, receives the memory stream. The caller
 *                      must close it after destroying the document.
 *
 * @return The parsed document, or @c NULL on failure.
 */
static CosDoc * COS_Nullable
parse_pdf_(const char *input,
           CosMemoryStream * COS_Nullable * COS_Nullable out_stream)
{
    CosDoc *doc = NULL;
    CosMemoryStream *stream = NULL;
    CosParser *parser = NULL;
    CosError error = cos_error_none();

    doc = cos_doc_create(NULL);
    if (!doc) {
        goto failure;
    }

    stream = cos_memory_stream_create_readonly(input, strlen(input));
    if (!stream) {
        goto failure;
    }

    parser = cos_parser_create(doc, (CosStream *)stream);
    if (!parser) {
        goto failure;
    }
    /* parser is now owned by doc; cos_doc_destroy will free it. */

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

// MARK: - Test PDF

/*
 * PDF with one indirect integer object: 1 0 obj 42 endobj
 *
 * Byte layout:
 *   offset   0 : %PDF-1.0\n              (9 bytes)
 *   offset   9 : 1 0 obj\n42\nendobj\n   (18 bytes)
 *   offset  27 : xref\n                  (5 bytes)
 *   offset  32 : 0 2\n                   (4 bytes)
 *   offset  36 : 0000000000 65535 f \n   (20 bytes)
 *   offset  56 : 0000000009 00000 n \n   (20 bytes)
 *   offset  76 : trailer\n               (8 bytes)
 *   offset  84 : << /Size 2 >>\n         (14 bytes)
 *   offset  98 : startxref\n             (10 bytes)
 *   offset 108 : 27\n                    (3 bytes)
 *   offset 111 : %%EOF                   (5 bytes)
 */
static const char k_pdf_one_indirect_obj[] =
    "%PDF-1.0\n"
    "1 0 obj\n42\nendobj\n"
    "xref\n"
    "0 2\n"
    "0000000000 65535 f \n"
    "0000000009 00000 n \n"
    "trailer\n"
    "<< /Size 2 >>\n"
    "startxref\n"
    "27\n"
    "%%EOF";

// MARK: - Tests

static int
resolveIndirectObj_MatchingGenNumber_ReturnsObject(void)
{
    CosMemoryStream *stream = NULL;
    CosDoc *doc = parse_pdf_(k_pdf_one_indirect_obj, &stream);
    TEST_EXPECT(doc != NULL);

    CosError error = cos_error_none();
    const CosObjID obj_id = cos_obj_id_make(1, 0);
    CosIndirectObjNode *indirect = cos_doc_get_object(doc, obj_id, &error);

    TEST_EXPECT(indirect != NULL);
    TEST_EXPECT(error.code == COS_ERROR_NONE);
    TEST_EXPECT(cos_obj_node_get_type((CosObjNode *)indirect) == CosObjNodeType_Indirect);

    CosObjNode *value = cos_indirect_obj_node_get_value(indirect);
    TEST_EXPECT(value != NULL);
    TEST_EXPECT(cos_obj_node_get_type(value) == CosObjNodeType_Integer);
    TEST_EXPECT(cos_int_obj_node_get_value((CosIntObjNode *)value) == 42);

    cos_obj_node_free((CosObjNode *)indirect);
    cos_doc_destroy(doc);
    cos_stream_close((CosStream *)stream);

    return EXIT_SUCCESS;
}

static int
resolveIndirectObj_MismatchedGenNumber_ReturnsError(void)
{
    CosMemoryStream *stream = NULL;
    CosDoc *doc = parse_pdf_(k_pdf_one_indirect_obj, &stream);
    TEST_EXPECT(doc != NULL);

    CosError error = cos_error_none();
    /* Object 1 is gen 0 in the xref; request it with gen 1. */
    const CosObjID obj_id = cos_obj_id_make(1, 1);
    CosObjNode *obj = cos_doc_get_object(doc, obj_id, &error);

    TEST_EXPECT(obj == NULL);
    TEST_EXPECT(error.code == COS_ERROR_XREF);

    cos_doc_destroy(doc);
    cos_stream_close((CosStream *)stream);

    return EXIT_SUCCESS;
}

static int
resolveIndirectObj_NonexistentObjNumber_ReturnsError(void)
{
    CosMemoryStream *stream = NULL;
    CosDoc *doc = parse_pdf_(k_pdf_one_indirect_obj, &stream);
    TEST_EXPECT(doc != NULL);

    CosError error = cos_error_none();
    /* Object 99 does not exist in the xref table. */
    const CosObjID obj_id = cos_obj_id_make(99, 0);
    CosObjNode *obj = cos_doc_get_object(doc, obj_id, &error);

    TEST_EXPECT(obj == NULL);
    TEST_EXPECT(error.code == COS_ERROR_XREF);

    cos_doc_destroy(doc);
    cos_stream_close((CosStream *)stream);

    return EXIT_SUCCESS;
}

static int
resolveIndirectObj_FreeEntry_ReturnsError(void)
{
    CosMemoryStream *stream = NULL;
    CosDoc *doc = parse_pdf_(k_pdf_one_indirect_obj, &stream);
    TEST_EXPECT(doc != NULL);

    CosError error = cos_error_none();
    /* Object 0 is always the head of the free list. */
    const CosObjID obj_id = cos_obj_id_make(0, 65535);
    CosObjNode *obj = cos_doc_get_object(doc, obj_id, &error);

    TEST_EXPECT(obj == NULL);
    TEST_EXPECT(error.code == COS_ERROR_XREF);

    cos_doc_destroy(doc);
    cos_stream_close((CosStream *)stream);

    return EXIT_SUCCESS;
}

// MARK: - Test driver

TEST_MAIN()
{
    TEST_EXPECT(resolveIndirectObj_MatchingGenNumber_ReturnsObject() == EXIT_SUCCESS);
    TEST_EXPECT(resolveIndirectObj_MismatchedGenNumber_ReturnsError() == EXIT_SUCCESS);
    TEST_EXPECT(resolveIndirectObj_NonexistentObjNumber_ReturnsError() == EXIT_SUCCESS);
    TEST_EXPECT(resolveIndirectObj_FreeEntry_ReturnsError() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END
