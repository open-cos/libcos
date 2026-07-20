/*
 * Copyright (c) 2026 OpenCOS.
 */

#include "CosTest.h"

#include <libcos/CosDoc.h>
#include <libcos/CosObjID.h>
#include <libcos/CosParser.h>
#include <libcos/common/CosError.h>
#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>
#include <libcos/objects/CosIndirectObjNode.h>
#include <libcos/objects/CosObjNode.h>
#include <libcos/objects/CosStreamObjNode.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

/*
 * Parses a full in-memory PDF and returns the document. The stream is returned
 * through out_stream so the caller can keep it alive for the document's use.
 */
static CosDoc * COS_Nullable
parse_pdf_bytes_(const char *bytes,
                 size_t size,
                 CosMemoryStream * COS_Nullable * COS_Nonnull out_stream)
{
    *out_stream = NULL;

    CosDoc *doc = cos_doc_create();
    if (!doc) {
        return NULL;
    }

    CosMemoryStream * const stream = cos_memory_stream_create_readonly(bytes, size);
    if (!stream) {
        cos_doc_destroy(doc);
        return NULL;
    }

    CosParser * const parser = cos_parser_create(doc, (CosStream *)stream, NULL);
    if (!parser) {
        cos_stream_close((CosStream *)stream);
        cos_doc_destroy(doc);
        return NULL;
    }

    if (!cos_parser_parse(parser, NULL)) {
        cos_stream_close((CosStream *)stream);
        cos_doc_destroy(doc);
        return NULL;
    }

    *out_stream = stream;
    return doc;
}

/*
 * Builds a classic-xref PDF whose object 1 is a stream with no /Length and whose
 * data embeds the literal bytes "endstream". Object 2 follows it so that the
 * cross-reference table gives an upper bound for the recovery scan. Offsets are
 * computed here rather than hard-coded.
 */
static size_t
build_embedded_endstream_pdf_(char *buf,
                              size_t cap,
                              const char *data)
{
    size_t n = 0;
    n += (size_t)snprintf(buf + n, cap - n, "%%PDF-1.7\n");

    const size_t obj1_off = n;
    n += (size_t)snprintf(buf + n, cap - n,
                          "1 0 obj\n<< >>\nstream\n%s\nendstream\nendobj\n",
                          data);

    const size_t obj2_off = n;
    n += (size_t)snprintf(buf + n, cap - n, "2 0 obj\nnull\nendobj\n");

    const size_t xref_off = n;
    n += (size_t)snprintf(buf + n, cap - n,
                          "xref\n"
                          "0 3\n"
                          "0000000000 65535 f \n"
                          "%010zu 00000 n \n"
                          "%010zu 00000 n \n"
                          "trailer\n<< /Size 3 >>\n"
                          "startxref\n%zu\n"
                          "%%%%EOF",
                          obj1_off, obj2_off, xref_off);

    return n;
}

static int
recover_boundedBackwardScan_BeatsEmbeddedEndstream(void)
{
    // 13 bytes, embedding the literal keyword a forward scan would stop at.
    static const char data[] = "AAendstreamBB";
    const size_t data_len = sizeof(data) - 1;

    char buf[512];
    const size_t size = build_embedded_endstream_pdf_(buf, sizeof(buf), data);
    TEST_EXPECT(size < sizeof(buf));

    CosMemoryStream *stream = NULL;
    CosDoc * const doc = parse_pdf_bytes_(buf, size, &stream);
    TEST_EXPECT(doc != NULL);

    // /Length is absent, so recovery runs even under the default Trust behaviour.
    // With the xref table available, it is bounded by object 2 and scans backward
    // to the real terminator, recovering all 13 data bytes rather than the 2 a
    // forward scan would yield by stopping at the embedded "endstream".
    CosError error = cos_error_none();
    CosObjNode * const indirect =
        cos_doc_get_object(doc, cos_obj_id_make(1, 0), &error);
    TEST_EXPECT(indirect != NULL);
    TEST_EXPECT(error.code == COS_ERROR_NONE);
    TEST_EXPECT(cos_obj_node_get_type(indirect) == CosObjNodeType_Indirect);

    CosObjNode * const value =
        cos_indirect_obj_node_get_value((CosIndirectObjNode *)indirect);
    TEST_EXPECT(value != NULL);
    TEST_EXPECT(cos_obj_node_get_type(value) == CosObjNodeType_Stream);
    TEST_EXPECT(cos_stream_obj_node_get_length((CosStreamObjNode *)value) == data_len);

    cos_obj_node_release(indirect);
    cos_doc_destroy(doc);
    cos_stream_close((CosStream *)stream);
    return EXIT_SUCCESS;
}

TEST_MAIN()
{
    TEST_EXPECT(recover_boundedBackwardScan_BeatsEmbeddedEndstream() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END
