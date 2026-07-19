/*
 * Copyright (c) 2026 OpenCOS.
 */

#include "CosTest.h"

#include <libcos/CosDoc.h>
#include <libcos/CosParser.h>
#include <libcos/common/CosError.h>
#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

// Parses @p bytes, reporting whether the parse succeeded and the resulting error code.
static bool
parse_pdf_(const char *bytes,
           size_t size,
           CosErrorCode *out_code)
{
    CosDoc *doc = cos_doc_create(NULL);
    CosMemoryStream *stream = NULL;
    CosParser *parser = NULL;
    CosError error = cos_error_none();
    bool result = false;

    if (!doc) {
        goto done;
    }

    stream = cos_memory_stream_create_readonly((const unsigned char *)bytes, size);
    if (!stream) {
        goto done;
    }

    parser = cos_parser_create(doc, (CosStream *)stream, NULL);
    if (!parser) {
        goto done;
    }

    result = cos_parser_parse(parser, &error);
    *out_code = error.code;

done:
    if (doc) {
        cos_doc_destroy(doc);
    }
    if (stream) {
        cos_stream_close((CosStream *)stream);
    }
    return result;
}

// A single-revision classic-table document whose trailer declares /Encrypt.
static const char encrypted_pdf[] =
    "%PDF-1.5\n"
    "1 0 obj\n"
    "<< /Type /Catalog >>\n"
    "endobj\n"
    "xref\n"
    "0 2\n"
    "0000000000 65535 f \n"
    "0000000009 00000 n \n"
    "trailer\n"
    "<< /Size 2 /Root 1 0 R /Encrypt 9 0 R >>\n"
    "startxref\n"
    "45\n"
    "%%EOF";

// The same document without /Encrypt, so that the check cannot pass by rejecting everything.
static const char plain_pdf[] =
    "%PDF-1.5\n"
    "1 0 obj\n"
    "<< /Type /Catalog >>\n"
    "endobj\n"
    "xref\n"
    "0 2\n"
    "0000000000 65535 f \n"
    "0000000009 00000 n \n"
    "trailer\n"
    "<< /Size 2 /Root 1 0 R >>\n"
    "startxref\n"
    "45\n"
    "%%EOF";

static int
encrypted_document_is_rejected(void)
{
    CosErrorCode code = COS_ERROR_NONE;
    TEST_EXPECT(!parse_pdf_(encrypted_pdf, sizeof(encrypted_pdf) - 1, &code));
    TEST_EXPECT(code == COS_ERROR_NOT_IMPLEMENTED);
    return EXIT_SUCCESS;
}

static int
plain_document_still_parses(void)
{
    CosErrorCode code = COS_ERROR_NONE;
    TEST_EXPECT(parse_pdf_(plain_pdf, sizeof(plain_pdf) - 1, &code));
    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END

TEST_MAIN()
{
    TEST_EXPECT(encrypted_document_is_rejected() == EXIT_SUCCESS);
    TEST_EXPECT(plain_document_still_parses() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}
