/*
 * Copyright (c) 2025 OpenCOS.
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

/**
 * @brief Runs a full PDF parse against an in-memory string.
 *
 * Creates a document and parser, executes @c cos_parser_parse(), then tears
 * everything down.  The caller receives the parse result and, optionally, the
 * error that was set on failure.
 *
 * @param input  NUL-terminated PDF bytes.
 * @param out_error  Written on failure; may be @c NULL.
 *
 * @return @c true if the parse succeeded, @c false otherwise.
 */
static bool
parse_pdf_(const char *input,
           CosError * COS_Nullable out_error)
{
    CosDoc *doc = NULL;
    CosMemoryStream *stream = NULL;
    CosParser *parser = NULL;
    bool result = false;

    doc = cos_doc_create(NULL);
    if (!doc) {
        goto cleanup;
    }

    stream = cos_memory_stream_create_readonly(input,
                                               strlen(input));
    if (!stream) {
        goto cleanup;
    }

    parser = cos_parser_create(doc, (CosStream *)stream);
    if (!parser) {
        goto cleanup;
    }
    /* parser is now owned by doc; cos_doc_destroy will free it. */

    result = cos_parser_parse(parser, out_error);

cleanup:
    if (doc) {
        cos_doc_destroy(doc);
    }
    if (stream) {
        cos_stream_close((CosStream *)stream);
    }
    return result;
}

// MARK: - Happy-path test

/*
 * Minimal but structurally complete PDF.
 *
 * Byte layout (used to compute startxref offset = 9):
 *   offset  0 : %PDF-1.0\n            (9 bytes)
 *   offset  9 : xref\n                (5 bytes)
 *   offset 14 : 0 1\n                 (4 bytes)
 *   offset 18 : 0000000000 65535 f \n (20 bytes)
 *   offset 38 : trailer\n             (8 bytes)
 *   offset 46 : << /Size 1 >>\n       (14 bytes)
 *   offset 60 : startxref\n           (10 bytes)
 *   offset 70 : 9\n                   (2 bytes)
 *   offset 72 : %%EOF                 (5 bytes)
 */
static const char k_minimal_valid_pdf[] =
    "%PDF-1.0\n"
    "xref\n"
    "0 1\n"
    "0000000000 65535 f \n"
    "trailer\n"
    "<< /Size 1 >>\n"
    "startxref\n"
    "9\n"
    "%%EOF";

static int
parse_minimalValidPdf_Succeeds(void)
{
    CosError error = cos_error_none();
    const bool result = parse_pdf_(k_minimal_valid_pdf, &error);

    TEST_EXPECT(result);
    TEST_EXPECT(error.code == COS_ERROR_NONE);

    return EXIT_SUCCESS;
}

// MARK: - Header format tests

static int
parse_fileTooShort_ReturnsError(void)
{
    /* 7 bytes — shorter than the 8-byte minimum for "%PDF-X.Y". */
    CosError error = cos_error_none();
    const bool result = parse_pdf_("%PDF-1.", &error);

    TEST_EXPECT(!result);
    TEST_EXPECT(error.code == COS_ERROR_SYNTAX);

    return EXIT_SUCCESS;
}

static int
parse_missingPdfPrefix_ReturnsError(void)
{
    /* Correct length but wrong magic bytes. */
    CosError error = cos_error_none();
    const bool result = parse_pdf_("%PSF-1.0\n", &error);

    TEST_EXPECT(!result);
    TEST_EXPECT(error.code == COS_ERROR_SYNTAX);

    return EXIT_SUCCESS;
}

static int
parse_invalidVersionNoDot_ReturnsError(void)
{
    /* "%PDF-17" — dot separator is missing (header[6] == '7', not '.'). */
    CosError error = cos_error_none();
    const bool result = parse_pdf_("%PDF-17\n", &error);

    TEST_EXPECT(!result);
    TEST_EXPECT(error.code == COS_ERROR_SYNTAX);

    return EXIT_SUCCESS;
}

static int
parse_invalidVersionMajorNotDigit_ReturnsError(void)
{
    /* Major version is a letter, not a digit. */
    CosError error = cos_error_none();
    const bool result = parse_pdf_("%PDF-A.7\n", &error);

    TEST_EXPECT(!result);
    TEST_EXPECT(error.code == COS_ERROR_SYNTAX);

    return EXIT_SUCCESS;
}

static int
parse_invalidVersionMinorNotDigit_ReturnsError(void)
{
    /* Minor version is a letter, not a digit. */
    CosError error = cos_error_none();
    const bool result = parse_pdf_("%PDF-1.B\n", &error);

    TEST_EXPECT(!result);
    TEST_EXPECT(error.code == COS_ERROR_SYNTAX);

    return EXIT_SUCCESS;
}

// MARK: - EOF marker tests

static int
parse_missingEofMarker_ReturnsError(void)
{
    /* Well-formed header and xref/trailer, but no %%EOF at all. */
    CosError error = cos_error_none();
    const bool result = parse_pdf_(
        "%PDF-1.0\n"
        "xref\n"
        "0 1\n"
        "0000000000 65535 f \n"
        "trailer\n"
        "<< /Size 1 >>\n"
        "startxref\n"
        "9\n",
        &error);

    TEST_EXPECT(!result);
    TEST_EXPECT(error.code == COS_ERROR_SYNTAX);

    return EXIT_SUCCESS;
}

static int
parse_eofMarkerMalformed_ReturnsError(void)
{
    /*
     * "%EOF" uses a single '%', not "%%EOF".  The backward scanner
     * searches for the five-byte sequence "%%EOF" and will not find it.
     */
    CosError error = cos_error_none();
    const bool result = parse_pdf_(
        "%PDF-1.0\n"
        "xref\n"
        "0 1\n"
        "0000000000 65535 f \n"
        "trailer\n"
        "<< /Size 1 >>\n"
        "startxref\n"
        "9\n"
        "%EOF",
        &error);

    TEST_EXPECT(!result);
    TEST_EXPECT(error.code == COS_ERROR_SYNTAX);

    return EXIT_SUCCESS;
}

// MARK: - startxref / xref-offset tests

static int
parse_missingXrefOffset_ReturnsError(void)
{
    /*
     * "startxref" is present but there is no integer between it and "%%EOF".
     * The backward scanner expects a decimal offset immediately before "%%EOF".
     */
    CosError error = cos_error_none();
    const bool result = parse_pdf_(
        "%PDF-1.0\n"
        "xref\n"
        "0 1\n"
        "0000000000 65535 f \n"
        "trailer\n"
        "<< /Size 1 >>\n"
        "startxref\n"
        "%%EOF",
        &error);

    TEST_EXPECT(!result);
    TEST_EXPECT(error.code == COS_ERROR_SYNTAX);

    return EXIT_SUCCESS;
}

static int
parse_missingStartxrefKeyword_ReturnsError(void)
{
    /*
     * An integer is present before "%%EOF" but the "startxref" keyword that
     * must precede it is absent.
     *
     * Byte layout:
     *   offset  0 : %PDF-1.0\n            (9 bytes)
     *   offset  9 : xref\n                (5 bytes)
     *   offset 14 : 0 1\n                 (4 bytes)
     *   offset 18 : 0000000000 65535 f \n (20 bytes)
     *   offset 38 : trailer\n             (8 bytes)
     *   offset 46 : << /Size 1 >>\n       (14 bytes)
     *   offset 60 : 9\n                   (2 bytes) <- no "startxref" before this
     *   offset 62 : %%EOF                 (5 bytes)
     */
    CosError error = cos_error_none();
    const bool result = parse_pdf_(
        "%PDF-1.0\n"
        "xref\n"
        "0 1\n"
        "0000000000 65535 f \n"
        "trailer\n"
        "<< /Size 1 >>\n"
        "9\n"
        "%%EOF",
        &error);

    TEST_EXPECT(!result);
    TEST_EXPECT(error.code == COS_ERROR_SYNTAX);

    return EXIT_SUCCESS;
}

// MARK: - Out-of-order structure tests

static int
parse_xrefNotBeforeTrailer_ReturnsError(void)
{
    /*
     * The startxref offset (9) points to the "trailer" keyword, not to the
     * "xref" keyword.  In this file the trailer section appears first in the
     * stream, followed by the xref table — the inverse of the required order.
     *
     * Byte layout (startxref = 9):
     *   offset  0 : %PDF-1.0\n            (9 bytes)
     *   offset  9 : trailer\n             (8 bytes)  ← startxref points here
     *   offset 17 : << /Size 1 >>\n       (14 bytes)
     *   offset 31 : xref\n                (5 bytes)
     *   offset 36 : 0 1\n                 (4 bytes)
     *   offset 40 : 0000000000 65535 f \n (20 bytes)
     *   offset 60 : startxref\n           (10 bytes)
     *   offset 70 : 9\n                   (2 bytes)
     *   offset 72 : %%EOF                 (5 bytes)
     */
    CosError error = cos_error_none();
    const bool result = parse_pdf_(
        "%PDF-1.0\n"
        "trailer\n"
        "<< /Size 1 >>\n"
        "xref\n"
        "0 1\n"
        "0000000000 65535 f \n"
        "startxref\n"
        "9\n"
        "%%EOF",
        &error);

    TEST_EXPECT(!result);
    TEST_EXPECT(error.code == COS_ERROR_XREF);

    return EXIT_SUCCESS;
}

static int
parse_trailerDictMissingBeforeEof_ReturnsError(void)
{
    /*
     * The xref table is valid, the "trailer" keyword is present, but no
     * dictionary follows it — "startxref" appears immediately instead.
     * The object parser cannot produce a dict from a "startxref" token and
     * returns NULL, causing the parse to fail.
     *
     * Byte layout (startxref = 9):
     *   offset  0 : %PDF-1.0\n            (9 bytes)
     *   offset  9 : xref\n                (5 bytes)
     *   offset 14 : 0 1\n                 (4 bytes)
     *   offset 18 : 0000000000 65535 f \n (20 bytes)
     *   offset 38 : trailer\n             (8 bytes)
     *   offset 46 : startxref\n           (10 bytes)  ← dict absent
     *   offset 56 : 9\n                   (2 bytes)
     *   offset 58 : %%EOF                 (5 bytes)
     */
    CosError error = cos_error_none();
    const bool result = parse_pdf_(
        "%PDF-1.0\n"
        "xref\n"
        "0 1\n"
        "0000000000 65535 f \n"
        "trailer\n"
        "startxref\n"
        "9\n"
        "%%EOF",
        &error);

    TEST_EXPECT(!result);

    return EXIT_SUCCESS;
}

// MARK: - Trailer content tests

static int
parse_trailerObjectNotDict_ReturnsError(void)
{
    /*
     * The xref table is valid and the "trailer" keyword is present, but the
     * following object is an integer (42), not a dictionary.
     *
     * Byte layout (startxref = 9):
     *   offset  0 : %PDF-1.0\n            (9 bytes)
     *   offset  9 : xref\n                (5 bytes)
     *   offset 14 : 0 1\n                 (4 bytes)
     *   offset 18 : 0000000000 65535 f \n (20 bytes)
     *   offset 38 : trailer\n             (8 bytes)
     *   offset 46 : 42\n                  (3 bytes)  ← integer, not dict
     *   offset 49 : startxref\n           (10 bytes)
     *   offset 59 : 9\n                   (2 bytes)
     *   offset 61 : %%EOF                 (5 bytes)
     */
    CosError error = cos_error_none();
    const bool result = parse_pdf_(
        "%PDF-1.0\n"
        "xref\n"
        "0 1\n"
        "0000000000 65535 f \n"
        "trailer\n"
        "42\n"
        "startxref\n"
        "9\n"
        "%%EOF",
        &error);

    TEST_EXPECT(!result);
    TEST_EXPECT(error.code == COS_ERROR_PARSE);

    return EXIT_SUCCESS;
}

// MARK: - Test driver

TEST_MAIN()
{
    /* Happy-path */
    TEST_EXPECT(parse_minimalValidPdf_Succeeds() == EXIT_SUCCESS);

    /* Header format */
    TEST_EXPECT(parse_fileTooShort_ReturnsError() == EXIT_SUCCESS);
    TEST_EXPECT(parse_missingPdfPrefix_ReturnsError() == EXIT_SUCCESS);
    TEST_EXPECT(parse_invalidVersionNoDot_ReturnsError() == EXIT_SUCCESS);
    TEST_EXPECT(parse_invalidVersionMajorNotDigit_ReturnsError() == EXIT_SUCCESS);
    TEST_EXPECT(parse_invalidVersionMinorNotDigit_ReturnsError() == EXIT_SUCCESS);

    /* EOF marker format */
    TEST_EXPECT(parse_missingEofMarker_ReturnsError() == EXIT_SUCCESS);
    TEST_EXPECT(parse_eofMarkerMalformed_ReturnsError() == EXIT_SUCCESS);

    /* startxref / xref-offset */
    TEST_EXPECT(parse_missingXrefOffset_ReturnsError() == EXIT_SUCCESS);
    TEST_EXPECT(parse_missingStartxrefKeyword_ReturnsError() == EXIT_SUCCESS);

    /* Out-of-order structure */
    TEST_EXPECT(parse_xrefNotBeforeTrailer_ReturnsError() == EXIT_SUCCESS);
    TEST_EXPECT(parse_trailerDictMissingBeforeEof_ReturnsError() == EXIT_SUCCESS);

    /* Trailer content */
    TEST_EXPECT(parse_trailerObjectNotDict_ReturnsError() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END
