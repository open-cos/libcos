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
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

// Parses @p bytes, expecting the parse to fail. Reports the error through @p out_error, which is
// left as COS_ERROR_NONE if the parse unexpectedly succeeded.
static void
parse_expecting_failure_(const char *bytes,
                         size_t size,
                         CosError *out_error)
{
    CosDoc *doc = cos_doc_create(NULL);
    CosMemoryStream *stream = NULL;
    CosParser *parser = NULL;
    CosError error = cos_error_none();

    *out_error = cos_error_none();

    if (!doc) {
        goto done;
    }

    stream = cos_memory_stream_create_readonly((const unsigned char *)bytes, size);
    if (!stream) {
        goto done;
    }

    parser = cos_parser_create(doc, (CosStream *)stream);
    if (!parser) {
        goto done;
    }

    if (!cos_parser_parse(parser, &error)) {
        *out_error = error;
    }

done:
    if (doc) {
        cos_doc_destroy(doc);
    }
    if (stream) {
        cos_stream_close((CosStream *)stream);
    }
}

// Whether the error is the one raised by cycle detection specifically.
//
// The revision cap raises COS_ERROR_XREF for a cyclic chain too, after walking the cycle 1024
// times, so the code alone cannot tell the two guards apart. Matching the message keeps these
// tests pinned to cycle detection rather than silently degrading into cap tests.
static bool
is_cycle_error_(const CosError *error)
{
    return error->code == COS_ERROR_XREF &&
           error->message != NULL &&
           strstr(error->message, "cycle") != NULL;
}

// A classic xref table at offset 45 whose trailer /Prev points back at the table itself. The only
// offset that has to be right is the table's own, which is also the startxref value.
static const char self_referential_pdf[] =
    "%PDF-1.5\n"
    "1 0 obj\n"
    "<< /Type /Catalog >>\n"
    "endobj\n"
    "xref\n"
    "0 2\n"
    "0000000000 65535 f \n"
    "0000000009 00000 n \n"
    "trailer\n"
    "<< /Size 2 /Root 1 0 R /Prev 45 >>\n"
    "startxref\n"
    "45\n"
    "%%EOF";

// Two classic xref tables that name each other: the table at 45 has /Prev 138, and the table at
// 138 has /Prev 45. startxref enters the cycle at 45.
static const char mutual_cycle_pdf[] =
    "%PDF-1.5\n"
    "1 0 obj\n"
    "<< /Type /Catalog >>\n"
    "endobj\n"
    // Offset 45: revision A, 93 bytes long, so revision B begins at 138.
    "xref\n"
    "0 2\n"
    "0000000000 65535 f \n"
    "0000000009 00000 n \n"
    "trailer\n"
    "<< /Size 2 /Root 1 0 R /Prev 138 >>\n"
    // Offset 138: revision B, pointing back at revision A.
    "xref\n"
    "0 2\n"
    "0000000000 65535 f \n"
    "0000000009 00000 n \n"
    "trailer\n"
    "<< /Size 2 /Root 1 0 R /Prev 45 >>\n"
    "startxref\n"
    "45\n"
    "%%EOF";

static int
self_referential_prev_is_rejected(void)
{
    // Guards the hand-counted offsets baked into the fixture: the table must really start at 45.
    TEST_EXPECT(memcmp(self_referential_pdf + 45, "xref", 4) == 0);

    CosError error = cos_error_none();
    parse_expecting_failure_(self_referential_pdf, sizeof(self_referential_pdf) - 1, &error);
    TEST_EXPECT(is_cycle_error_(&error));
    return EXIT_SUCCESS;
}

static int
mutual_prev_cycle_is_rejected(void)
{
    TEST_EXPECT(memcmp(mutual_cycle_pdf + 45, "xref", 4) == 0);
    TEST_EXPECT(memcmp(mutual_cycle_pdf + 138, "xref", 4) == 0);

    CosError error = cos_error_none();
    parse_expecting_failure_(mutual_cycle_pdf, sizeof(mutual_cycle_pdf) - 1, &error);
    TEST_EXPECT(is_cycle_error_(&error));
    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END

TEST_MAIN()
{
    TEST_EXPECT(self_referential_prev_is_rejected() == EXIT_SUCCESS);
    TEST_EXPECT(mutual_prev_cycle_is_rejected() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}
