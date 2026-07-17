/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "CosTest.h"

#include "parse/CosObjParser.h"

#include <libcos/CosDoc.h>
#include <libcos/common/CosData.h>
#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>
#include <libcos/objects/CosObjNode.h>
#include <libcos/objects/CosStringObjNode.h>

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

// MARK: - String tests

/**
 * Parses @p input and returns its string value, or @c NULL if the first object
 * is not a string. The returned data belongs to @p out_node, which the caller
 * releases.
 */
static const CosData * COS_Nullable
parse_string_value_(const char *input,
                    CosDoc * COS_Nullable * COS_Nonnull out_doc,
                    CosMemoryStream * COS_Nullable * COS_Nonnull out_stream,
                    CosObjNode * COS_Nullable * COS_Nonnull out_node)
{
    *out_doc = NULL;
    *out_stream = NULL;
    *out_node = NULL;

    CosDoc * const doc = cos_doc_create(NULL);
    if (!doc) {
        return NULL;
    }
    *out_doc = doc;

    CosMemoryStream * const stream = cos_memory_stream_create_readonly(input, strlen(input));
    if (!stream) {
        return NULL;
    }
    *out_stream = stream;

    CosObjParser * const parser = cos_obj_parser_create(doc, (CosStream *)stream);
    if (!parser) {
        return NULL;
    }

    CosObjNode * const obj = cos_obj_parser_next_object(parser, NULL);
    cos_obj_parser_destroy(parser);
    if (!obj) {
        return NULL;
    }
    *out_node = obj;

    if (cos_obj_node_get_type(obj) != CosObjNodeType_String) {
        return NULL;
    }

    return cos_string_obj_node_get_value((CosStringObjNode *)obj);
}

#define STRING_TEST_BEGIN(input_)                                     \
    CosDoc *doc = NULL;                                               \
    CosMemoryStream *stream = NULL;                                   \
    CosObjNode *node = NULL;                                          \
    const CosData * const data =                                      \
        parse_string_value_((input_), &doc, &stream, &node)

#define STRING_TEST_END()                    \
    do {                                     \
        if (node) {                          \
            cos_obj_node_release(node);      \
        }                                    \
        if (doc) {                           \
            cos_doc_destroy(doc);            \
        }                                    \
        if (stream) {                        \
            cos_stream_close((CosStream *)stream); \
        }                                    \
    } while (0)

static bool
data_equals_(const CosData * COS_Nullable data,
             const char *expected,
             size_t expected_size)
{
    return data != NULL &&
           data->size == expected_size &&
           memcmp(data->bytes, expected, expected_size) == 0;
}

static int
parse_literalString_HasCorrectValue(void)
{
    STRING_TEST_BEGIN("(hello)");
    const bool ok = data_equals_(data, "hello", 5);
    STRING_TEST_END();
    TEST_EXPECT(ok);
    return EXIT_SUCCESS;
}

static int
parse_hexString_HasCorrectValue(void)
{
    /* A hex string is a string too: the guard has to accept both types. */
    STRING_TEST_BEGIN("<48656C6C6F>");
    const bool ok = data_equals_(data, "Hello", 5);
    STRING_TEST_END();
    TEST_EXPECT(ok);
    return EXIT_SUCCESS;
}

static int
parse_emptyLiteralString_HasEmptyValue(void)
{
    STRING_TEST_BEGIN("()");
    const bool ok = data_equals_(data, "", 0);
    STRING_TEST_END();
    TEST_EXPECT(ok);
    return EXIT_SUCCESS;
}

static int
parse_stringWithEmbeddedNul_KeepsAllBytes(void)
{
    STRING_TEST_BEGIN("(a\\000b)");
    const bool ok = data_equals_(data, "a\0b", 3);
    STRING_TEST_END();
    TEST_EXPECT(ok);
    return EXIT_SUCCESS;
}

static int
parse_stringInArray_IsAnElement(void)
{
    CosObjNodeType types[MAX_OBJECTS] = {CosObjNodeType_Unknown};
    TEST_EXPECT(parse_objects_("[ (a) <42> (b) ]", types) == 1);
    TEST_EXPECT(types[0] == CosObjNodeType_Array);
    return EXIT_SUCCESS;
}

static int
parse_stringsAreConsumed(void)
{
    CosObjNodeType types[MAX_OBJECTS] = {CosObjNodeType_Unknown};
    TEST_EXPECT(parse_objects_("(a) <42> (b)", types) == 3);
    TEST_EXPECT(types[0] == CosObjNodeType_String);
    TEST_EXPECT(types[1] == CosObjNodeType_String);
    TEST_EXPECT(types[2] == CosObjNodeType_String);
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

    /* Strings */
    TEST_EXPECT(parse_literalString_HasCorrectValue() == EXIT_SUCCESS);
    TEST_EXPECT(parse_hexString_HasCorrectValue() == EXIT_SUCCESS);
    TEST_EXPECT(parse_emptyLiteralString_HasEmptyValue() == EXIT_SUCCESS);
    TEST_EXPECT(parse_stringWithEmbeddedNul_KeepsAllBytes() == EXIT_SUCCESS);
    TEST_EXPECT(parse_stringInArray_IsAnElement() == EXIT_SUCCESS);
    TEST_EXPECT(parse_stringsAreConsumed() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END
