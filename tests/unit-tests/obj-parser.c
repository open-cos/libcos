/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "CosTest.h"

#include "parse/CosObjParser.h"

#include <libcos/CosDoc.h>
#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>
#include <libcos/objects/CosObjNode.h>

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

// MARK: - Helpers

enum {
    /**
     * The object count at which a test gives up.
     *
     * Every token has to be consumed by the object it produces. A parser that
     * returns an object without consuming its token would hand back the same
     * one forever, so the loop below is bounded: a test that reaches this
     * limit fails rather than hanging the suite.
     */
    MAX_OBJECTS = 16,
};

/**
 * Parses objects from @p input until the parser runs out.
 *
 * @param input The NUL-terminated input.
 * @param out_types Receives the type of each object parsed; at least
 *   @c MAX_OBJECTS elements.
 *
 * @return The number of objects parsed, or @c MAX_OBJECTS if the parser never
 *   ran out (which is itself a failure).
 */
static size_t
parse_objects_(const char *input,
               CosObjNodeType *out_types)
{
    CosDoc *doc = NULL;
    CosMemoryStream *stream = NULL;
    CosObjParser *parser = NULL;
    size_t count = 0;

    doc = cos_doc_create(NULL);
    if (!doc) {
        goto cleanup;
    }

    stream = cos_memory_stream_create_readonly(input, strlen(input));
    if (!stream) {
        goto cleanup;
    }

    parser = cos_obj_parser_create(doc, (CosStream *)stream);
    if (!parser) {
        goto cleanup;
    }

    while (count < MAX_OBJECTS && cos_obj_parser_has_next_object(parser)) {
        CosObjNode * const obj = cos_obj_parser_next_object(parser, NULL);
        if (!obj) {
            break;
        }

        out_types[count++] = cos_obj_node_get_type(obj);
        cos_obj_node_release(obj);
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
    return count;
}

// MARK: - Token consumption tests

static int
parse_null_ConsumesItsToken(void)
{
    CosObjNodeType types[MAX_OBJECTS] = {CosObjNodeType_Unknown};
    TEST_EXPECT(parse_objects_("null", types) == 1);
    TEST_EXPECT(types[0] == CosObjNodeType_Null);
    return EXIT_SUCCESS;
}

static int
parse_true_ConsumesItsToken(void)
{
    CosObjNodeType types[MAX_OBJECTS] = {CosObjNodeType_Unknown};
    TEST_EXPECT(parse_objects_("true", types) == 1);
    TEST_EXPECT(types[0] == CosObjNodeType_Boolean);
    return EXIT_SUCCESS;
}

static int
parse_false_ConsumesItsToken(void)
{
    CosObjNodeType types[MAX_OBJECTS] = {CosObjNodeType_Unknown};
    TEST_EXPECT(parse_objects_("false", types) == 1);
    TEST_EXPECT(types[0] == CosObjNodeType_Boolean);
    return EXIT_SUCCESS;
}

static int
parse_repeatedNulls_EachConsumedOnce(void)
{
    /* Three tokens must yield three objects, not the first one three times. */
    CosObjNodeType types[MAX_OBJECTS] = {CosObjNodeType_Unknown};
    TEST_EXPECT(parse_objects_("null null null", types) == 3);
    TEST_EXPECT(types[0] == CosObjNodeType_Null);
    TEST_EXPECT(types[1] == CosObjNodeType_Null);
    TEST_EXPECT(types[2] == CosObjNodeType_Null);
    return EXIT_SUCCESS;
}

static int
parse_mixedScalars_EachConsumedOnce(void)
{
    CosObjNodeType types[MAX_OBJECTS] = {CosObjNodeType_Unknown};
    TEST_EXPECT(parse_objects_("true 1 null false /N", types) == 5);
    TEST_EXPECT(types[0] == CosObjNodeType_Boolean);
    TEST_EXPECT(types[1] == CosObjNodeType_Integer);
    TEST_EXPECT(types[2] == CosObjNodeType_Null);
    TEST_EXPECT(types[3] == CosObjNodeType_Boolean);
    TEST_EXPECT(types[4] == CosObjNodeType_Name);
    return EXIT_SUCCESS;
}

static int
parse_nullInsideArray_Terminates(void)
{
    /*
     * The array rejects the null and stops, leaving the token for the caller.
     * It must still be consumed there rather than offered again forever.
     */
    CosObjNodeType types[MAX_OBJECTS] = {CosObjNodeType_Unknown};
    const size_t count = parse_objects_("[ null ]", types);
    TEST_EXPECT(count < MAX_OBJECTS);
    TEST_EXPECT(count > 0);
    TEST_EXPECT(types[0] == CosObjNodeType_Array);
    return EXIT_SUCCESS;
}

static int
parse_strayDictEndInNestedArray_Terminates(void)
{
    /* The 36-byte input that ran for over a minute. */
    CosObjNodeType types[MAX_OBJECTS] = {CosObjNodeType_Unknown};
    TEST_EXPECT(parse_objects_("1 0 obj\n[ 1 [2 [3 null] >> ]\nendobj\n", types) < MAX_OBJECTS);
    return EXIT_SUCCESS;
}

// MARK: - Test driver

TEST_MAIN()
{
    /* Token consumption */
    TEST_EXPECT(parse_null_ConsumesItsToken() == EXIT_SUCCESS);
    TEST_EXPECT(parse_true_ConsumesItsToken() == EXIT_SUCCESS);
    TEST_EXPECT(parse_false_ConsumesItsToken() == EXIT_SUCCESS);
    TEST_EXPECT(parse_repeatedNulls_EachConsumedOnce() == EXIT_SUCCESS);
    TEST_EXPECT(parse_mixedScalars_EachConsumedOnce() == EXIT_SUCCESS);
    TEST_EXPECT(parse_nullInsideArray_Terminates() == EXIT_SUCCESS);
    TEST_EXPECT(parse_strayDictEndInNestedArray_Terminates() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END
