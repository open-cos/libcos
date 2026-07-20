/*
 * Copyright (c) 2026 OpenCOS.
 */

#include "CosTest.h"
#include "support/CosFaultAllocator.h"

#include "parse/CosObjParser.h"

#include <libcos/CosDoc.h>
#include <libcos/common/CosArray.h>
#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>
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

/*
 * A full file-structure OOM test (CosParser over a complete PDF) is deliberately
 * not driven here yet. The parser is leak-safe under injection, but its xref and
 * trailer scanning legitimately tolerates a transient allocation failure in a
 * speculative peek (e.g. matches_next_token for the "trailer" keyword) and still
 * produces the correct result. Asserting "every injected fault fails the parse"
 * would therefore false-positive on those absorbed allocations; distinguishing
 * them needs SQLite-style benign-malloc regions, which are not yet implemented.
 */

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

TEST_MAIN()
{
    TEST_EXPECT(test_array_oom_transient() == EXIT_SUCCESS);
    TEST_EXPECT(test_array_oom_persistent() == EXIT_SUCCESS);

    TEST_EXPECT(test_objects_oom_transient() == EXIT_SUCCESS);
    TEST_EXPECT(test_objects_oom_persistent() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END
