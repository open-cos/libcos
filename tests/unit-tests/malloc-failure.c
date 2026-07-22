/*
 * Copyright (c) 2026 OpenCOS.
 */

#include "CosTest.h"
#include "support/CosFaultAllocator.h"

#include "parse/CosObjParser.h"

#include <libcos/CosDoc.h>
#include <libcos/CosParser.h>
#include <libcos/CosTrailer.h>
#include <libcos/common/CosArray.h>
#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>
#include <libcos/objects/CosDictObjNode.h>
#include <libcos/objects/CosObjNode.h>

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

// MARK: - CosArray

/*
 * The operation under test: build a CosArray, grow it past its initial capacity
 * (forcing a resize), read an item back, and tear it down. Every allocation --
 * the array struct, its data buffer, and the resize -- routes through the global
 * cos_* functions, so each is a distinct injection point while the fault
 * allocator is installed.
 *
 * It returns false the moment any allocation fails, always destroying whatever
 * it managed to create so the fault allocator's outstanding count returns to 0.
 */
static bool
oom_build_and_grow_array_(void * COS_Nullable ctx)
{
    (void)ctx;

    CosArray * const array = cos_array_create(sizeof(int), NULL, 4);
    if (!array) {
        return false;
    }

    bool ok = true;

    // 32 items forces at least one resize past the initial capacity of 4.
    for (int i = 0; i < 32; i++) {
        if (!cos_array_append_item(array, &i, NULL)) {
            ok = false;
            break;
        }
    }

    if (ok) {
        int value = -1;
        if (!cos_array_get_item(array, 10, &value, NULL) || value != 10) {
            ok = false;
        }
    }

    cos_array_destroy(array);
    return ok;
}

// MARK: - Strict options

/*
 * Parser options with every strict group escalated to Error.
 *
 * The parser tolerates several deviations by degrading to a diagnostic rather
 * than failing. Under injection, a fault on one of those lenient paths (a
 * missing endobj, a tolerated missing EOF marker) would otherwise leave the
 * object or document parseable, so the operation would report success while a
 * fault was active. Escalating every group to Error turns any such deviation
 * into a hard failure, so "parse succeeded" means "no injected fault altered the
 * outcome". The fixtures below are fully conforming, so a fault-free run still
 * parses cleanly at Error.
 */
static CosParserOptions
oom_strict_options_(void)
{
    CosParserOptions options = cos_parser_options_make_default();
    for (unsigned int i = 0; i < COS_STRICT_GROUP_COUNT; i++) {
        cos_parser_options_set_strict_level(&options,
                                            (CosStrictGroup)i,
                                            CosStrictLevel_Error);
    }
    return options;
}

// MARK: - Object parser

enum {
    /**
     * The number of top-level indirect objects in @c k_objects_input .
     *
     * The operation parses exactly this many rather than looping on
     * @c cos_obj_parser_has_next_object : under injection that predicate can
     * report "no more objects" because a peek allocation failed, which is
     * indistinguishable here from a genuine end of input. Requiring every
     * expected object to parse turns any injected failure into a short count.
     */
    EXPECTED_OBJECT_COUNT = 3,
};

/*
 * A small but varied body of COS syntax: three indirect object definitions
 * covering a dictionary with a reference value, an array holding a reference,
 * a literal string, a hex string, a real, a boolean, a null and a name, and a
 * stream whose /Length is correct (used as-is under the default behaviour, so
 * the parse stays silent). Exercising the tokenizer, object parser, object
 * nodes and stream handling, all of which allocate through cos_*.
 */
static const char k_objects_input[] =
    "1 0 obj\n"
    "<< /Type /Catalog /Pages 2 0 R >>\n"
    "endobj\n"
    "2 0 obj\n"
    "[ 1 0 R (hello) <48656C6C6F> 3.14 true null /Leaf ]\n"
    "endobj\n"
    "3 0 obj\n"
    "<< /Length 3 >>\n"
    "stream\n"
    "abc\n"
    "endstream\n"
    "endobj\n";

/*
 * The operation under test: parse every object in k_objects_input through a
 * CosObjParser and tear the whole pipeline down. Returns true only if all
 * EXPECTED_OBJECT_COUNT indirect objects parsed; any injected allocation
 * failure yields a NULL object (a short count, or a hard failure via the strict
 * options) and therefore false.
 */
static bool
oom_parse_objects_(void * COS_Nullable ctx)
{
    (void)ctx;

    CosDoc *doc = NULL;
    CosMemoryStream *stream = NULL;
    CosObjParser *parser = NULL;
    bool ok = false;

    doc = cos_doc_create();
    if (!doc) {
        goto cleanup;
    }

    stream = cos_memory_stream_create_readonly(k_objects_input,
                                               strlen(k_objects_input));
    if (!stream) {
        goto cleanup;
    }

    const CosParserOptions options = oom_strict_options_();
    parser = cos_obj_parser_create(doc, (CosStream *)stream, &options);
    if (!parser) {
        goto cleanup;
    }

    ok = true;
    for (size_t i = 0; i < EXPECTED_OBJECT_COUNT; i++) {
        CosObjNode * const obj = cos_obj_parser_next_object(parser, NULL);
        if (!obj) {
            ok = false;
            break;
        }
        if (cos_obj_node_get_type(obj) != CosObjNodeType_Indirect) {
            ok = false;
        }
        cos_obj_node_release(obj);
        if (!ok) {
            break;
        }
    }

cleanup:
    if (parser) {
        cos_obj_parser_destroy(parser);
    }
    if (stream) {
        cos_stream_close((CosStream *)stream);
    }
    if (doc) {
        cos_doc_destroy(doc);
    }
    return ok;
}

// MARK: - Document parser

/*
 * Minimal but structurally complete PDF: header, a one-entry xref table, a
 * trailer dictionary, startxref and %%EOF. Byte offsets are chosen so startxref
 * points at the "xref" keyword (offset 9); see tests/unit-tests/file-structure.c
 * for the annotated layout this mirrors.
 *
 * The xref and trailer scanning here reads speculatively, but an allocation
 * failure in those peeks now propagates (a NULL peek is "undetermined", not
 * "no token") rather than being absorbed, so an injected failure fails the
 * parse gracefully instead of returning a truncated result as success.
 */
static const char k_minimal_pdf[] =
    "%PDF-1.0\n"
    "xref\n"
    "0 1\n"
    "0000000000 65535 f \n"
    "trailer\n"
    "<< /Size 1 >>\n"
    "startxref\n"
    "9\n"
    "%%EOF";

/*
 * The operation under test: parse the complete file structure through a
 * CosParser and tear the whole pipeline down. Returns true only if the parse
 * succeeds AND yields a complete trailer (the /Size entry is present).
 *
 * The completeness check is what gives the test teeth. The dictionary and xref
 * scanners are lenient about genuine truncation (EOF or malformed input degrades
 * to "return what was parsed so far"), but an injected allocation failure now
 * propagates rather than being absorbed, so it fails the parse. Requiring /Size
 * also catches any residual path that returns a truncated trailer as a false
 * success.
 */
static bool
oom_parse_document_(void * COS_Nullable ctx)
{
    (void)ctx;

    CosDoc *doc = NULL;
    CosMemoryStream *stream = NULL;
    CosParser *parser = NULL;
    bool ok = false;

    doc = cos_doc_create();
    if (!doc) {
        goto cleanup;
    }

    stream = cos_memory_stream_create_readonly(k_minimal_pdf,
                                               strlen(k_minimal_pdf));
    if (!stream) {
        goto cleanup;
    }

    const CosParserOptions options = oom_strict_options_();
    parser = cos_parser_create(doc, (CosStream *)stream, &options);
    if (!parser) {
        goto cleanup;
    }
    // parser is now owned by doc; cos_doc_destroy will free it.

    if (!cos_parser_parse(parser, NULL)) {
        goto cleanup;
    }

    // Require a complete trailer: an absorbed allocation failure that truncated
    // the trailer dictionary drops the /Size entry, which must not pass as a
    // successful parse.
    const CosTrailer * const trailer = cos_doc_get_trailer(doc);
    if (!trailer) {
        goto cleanup;
    }
    const CosDictObjNode * const trailer_dict = cos_trailer_get_dict(trailer);
    CosObjNode *size_value = NULL;
    if (!trailer_dict ||
        !cos_dict_obj_node_get_value_with_string(trailer_dict, "Size", &size_value, NULL)) {
        goto cleanup;
    }

    ok = true;

cleanup:
    if (doc) {
        cos_doc_destroy(doc);
    }
    if (stream) {
        cos_stream_close((CosStream *)stream);
    }
    return ok;
}

// MARK: - Test driver

static int
test_array_oom_transient(void)
{
    return cos_oom_test_run(oom_build_and_grow_array_, NULL, COS_FAULT_TRANSIENT);
}

static int
test_array_oom_persistent(void)
{
    return cos_oom_test_run(oom_build_and_grow_array_, NULL, COS_FAULT_PERSISTENT);
}

static int
test_objects_oom_transient(void)
{
    return cos_oom_test_run(oom_parse_objects_, NULL, COS_FAULT_TRANSIENT);
}

static int
test_objects_oom_persistent(void)
{
    return cos_oom_test_run(oom_parse_objects_, NULL, COS_FAULT_PERSISTENT);
}

static int
test_document_oom_transient(void)
{
    return cos_oom_test_run(oom_parse_document_, NULL, COS_FAULT_TRANSIENT);
}

static int
test_document_oom_persistent(void)
{
    return cos_oom_test_run(oom_parse_document_, NULL, COS_FAULT_PERSISTENT);
}

TEST_MAIN()
{
    TEST_EXPECT(test_array_oom_transient() == EXIT_SUCCESS);
    TEST_EXPECT(test_array_oom_persistent() == EXIT_SUCCESS);

    TEST_EXPECT(test_objects_oom_transient() == EXIT_SUCCESS);
    TEST_EXPECT(test_objects_oom_persistent() == EXIT_SUCCESS);

    TEST_EXPECT(test_document_oom_transient() == EXIT_SUCCESS);
    TEST_EXPECT(test_document_oom_persistent() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END
