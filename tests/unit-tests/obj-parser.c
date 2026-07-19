/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "CosTest.h"

#include "parse/CosObjParser.h"

#include <libcos/CosDoc.h>
#include <libcos/common/CosData.h>
#include <libcos/common/CosDiagnostic.h>
#include <libcos/common/CosDiagnosticHandler.h>
#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>
#include <libcos/objects/CosArrayObjNode.h>
#include <libcos/objects/CosBoolObjNode.h>
#include <libcos/objects/CosDictObjNode.h>
#include <libcos/objects/CosObjNode.h>
#include <libcos/objects/CosStringObjNode.h>
#include <libcos/syntax/tokenizer/CosTokenizer.h>

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

    parser = cos_obj_parser_create(doc, (CosStream *)stream, NULL);
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
    /* The array accepts the null, then terminates cleanly at the closing "]". */
    CosObjNodeType types[MAX_OBJECTS] = {CosObjNodeType_Unknown};
    const size_t count = parse_objects_("[ null ]", types);
    TEST_EXPECT(count == 1);
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

    CosObjParser * const parser = cos_obj_parser_create(doc, (CosStream *)stream, NULL);
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

// MARK: - Context enforcement tests

enum {
    /** Returned by the container helpers when the first object is the wrong type. */
    NOT_A_CONTAINER = (size_t)-1,
};

/**
 * Parses the first object of @p input. If it is an array, copies each element's
 * type into @p out_types (up to @c MAX_OBJECTS) and returns the element count;
 * otherwise returns @c NOT_A_CONTAINER.
 */
static size_t
parse_array_element_types_(const char *input,
                           CosObjNodeType *out_types)
{
    CosDoc *doc = NULL;
    CosMemoryStream *stream = NULL;
    CosObjParser *parser = NULL;
    size_t count = NOT_A_CONTAINER;

    doc = cos_doc_create(NULL);
    if (!doc) {
        goto cleanup;
    }
    stream = cos_memory_stream_create_readonly(input, strlen(input));
    if (!stream) {
        goto cleanup;
    }
    parser = cos_obj_parser_create(doc, (CosStream *)stream, NULL);
    if (!parser) {
        goto cleanup;
    }

    CosObjNode * const obj = cos_obj_parser_next_object(parser, NULL);
    if (obj) {
        if (cos_obj_node_get_type(obj) == CosObjNodeType_Array) {
            const CosArrayObjNode * const array = (const CosArrayObjNode *)obj;
            count = cos_array_obj_node_get_count(array);
            for (size_t i = 0; i < count && i < MAX_OBJECTS; i++) {
                CosObjNode * const element = cos_array_obj_node_get_at(array, i, NULL);
                out_types[i] = element ? cos_obj_node_get_type(element) : CosObjNodeType_Unknown;
            }
        }
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

/**
 * Parses the first object of @p input and returns its entry count if it is a
 * dictionary, or @c NOT_A_CONTAINER otherwise.
 */
static size_t
parse_dict_entry_count_(const char *input)
{
    CosDoc *doc = NULL;
    CosMemoryStream *stream = NULL;
    CosObjParser *parser = NULL;
    size_t count = NOT_A_CONTAINER;

    doc = cos_doc_create(NULL);
    if (!doc) {
        goto cleanup;
    }
    stream = cos_memory_stream_create_readonly(input, strlen(input));
    if (!stream) {
        goto cleanup;
    }
    parser = cos_obj_parser_create(doc, (CosStream *)stream, NULL);
    if (!parser) {
        goto cleanup;
    }

    CosObjNode * const obj = cos_obj_parser_next_object(parser, NULL);
    if (obj) {
        if (cos_obj_node_get_type(obj) == CosObjNodeType_Dict) {
            count = cos_dict_obj_node_get_count((const CosDictObjNode *)obj);
        }
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

static int
parse_realInArray_IsElement(void)
{
    /* The inverted context check used to reject this, leaving the array empty. */
    CosObjNodeType types[MAX_OBJECTS] = {CosObjNodeType_Unknown};
    TEST_EXPECT(parse_array_element_types_("[ 3.14 ]", types) == 1);
    TEST_EXPECT(types[0] == CosObjNodeType_Real);
    return EXIT_SUCCESS;
}

static int
parse_nullInArray_IsElement(void)
{
    CosObjNodeType types[MAX_OBJECTS] = {CosObjNodeType_Unknown};
    TEST_EXPECT(parse_array_element_types_("[ null ]", types) == 1);
    TEST_EXPECT(types[0] == CosObjNodeType_Null);
    return EXIT_SUCCESS;
}

static int
parse_boolInArray_IsElement(void)
{
    CosObjNodeType types[MAX_OBJECTS] = {CosObjNodeType_Unknown};
    TEST_EXPECT(parse_array_element_types_("[ true ]", types) == 1);
    TEST_EXPECT(types[0] == CosObjNodeType_Boolean);
    return EXIT_SUCCESS;
}

static int
parse_mixedArray_KeepsOrder(void)
{
    CosObjNodeType types[MAX_OBJECTS] = {CosObjNodeType_Unknown};
    TEST_EXPECT(parse_array_element_types_("[ 1 3.14 2 ]", types) == 3);
    TEST_EXPECT(types[0] == CosObjNodeType_Integer);
    TEST_EXPECT(types[1] == CosObjNodeType_Real);
    TEST_EXPECT(types[2] == CosObjNodeType_Integer);
    return EXIT_SUCCESS;
}

static int
parse_referenceInArray_IsElement(void)
{
    /* A reference IS legal as an array element. */
    CosObjNodeType types[MAX_OBJECTS] = {CosObjNodeType_Unknown};
    TEST_EXPECT(parse_array_element_types_("[ 1 0 R ]", types) == 1);
    TEST_EXPECT(types[0] == CosObjNodeType_Reference);
    return EXIT_SUCCESS;
}

static int
parse_bareReference_DoesNotParse(void)
{
    /* A reference is NOT legal on its own at the top level -- only definitions. */
    CosObjNodeType types[MAX_OBJECTS] = {CosObjNodeType_Unknown};
    TEST_EXPECT(parse_objects_("1 0 R", types) == 0);
    return EXIT_SUCCESS;
}

static int
parse_dictWithNameKey_HasEntry(void)
{
    TEST_EXPECT(parse_dict_entry_count_("<< /A 1 >>") == 1);
    return EXIT_SUCCESS;
}

static int
parse_dictWithNonNameKey_DropsEntry(void)
{
    /* A non-name key is rejected; the entry never enters the dictionary. */
    TEST_EXPECT(parse_dict_entry_count_("<< 1 2 >>") == 0);
    return EXIT_SUCCESS;
}

// MARK: - Flush tests

static int
flush_DiscardsPeekedObject(void)
{
    /*
     * A peeked object belongs to the stream position it was parsed at. After a
     * seek and tokenizer reset, flushing must drop it, or next_object() would
     * hand back the stale peek instead of parsing at the new position. Input is
     * "true" at offset 0 and "false" at offset 5.
     */
    CosDoc *doc = NULL;
    CosMemoryStream *stream = NULL;
    CosTokenizer *tokenizer = NULL;
    CosObjParser *parser = NULL;
    int result = EXIT_FAILURE;

    doc = cos_doc_create(NULL);
    stream = cos_memory_stream_create_readonly("true false", strlen("true false"));
    if (!doc || !stream) {
        goto cleanup;
    }

    tokenizer = cos_tokenizer_create((CosStream *)stream, NULL);
    if (!tokenizer) {
        goto cleanup;
    }

    parser = cos_obj_parser_create_with_tokenizer(doc, tokenizer, NULL);
    if (!parser) {
        goto cleanup;
    }

    // Peek the first object ("true") at offset 0; the parser caches it.
    CosObjNode * const peeked = cos_obj_parser_peek_object(parser, NULL);
    if (!peeked ||
        cos_obj_node_get_type(peeked) != CosObjNodeType_Boolean ||
        cos_bool_obj_node_get_value((CosBoolObjNode *)peeked) != true) {
        goto cleanup;
    }

    // Reposition to "false" and flush, exactly as cos_parser_load_object does.
    if (!cos_stream_seek((CosStream *)stream, 5, CosStreamOffsetWhence_Set, NULL)) {
        goto cleanup;
    }
    cos_tokenizer_reset(tokenizer);
    cos_obj_parser_flush_tokens_(parser);

    // Without the flush fix this returns the stale "true".
    CosObjNode * const obj = cos_obj_parser_next_object(parser, NULL);
    if (obj &&
        cos_obj_node_get_type(obj) == CosObjNodeType_Boolean &&
        cos_bool_obj_node_get_value((CosBoolObjNode *)obj) == false) {
        result = EXIT_SUCCESS;
    }
    if (obj) {
        cos_obj_node_release(obj);
    }

cleanup:
    if (parser) {
        cos_obj_parser_destroy(parser);
    }
    if (tokenizer) {
        cos_tokenizer_destroy(tokenizer);
    }
    if (stream) {
        cos_stream_close((CosStream *)stream);
    }
    if (doc) {
        cos_doc_destroy(doc);
    }
    return result;
}

// MARK: - Strict mode helpers

/**
 * Counts the diagnostics emitted during a parse.
 */
typedef struct DiagnosticCounter {
    size_t warnings;
    size_t errors;
} DiagnosticCounter;

static void
count_diagnostic_(CosDiagnosticHandler *handler,
                  const CosDiagnostic *diagnostic)
{
    DiagnosticCounter * const counter =
        (DiagnosticCounter *)cos_diagnostic_handler_get_user_data(handler);
    if (!counter) {
        return;
    }

    if (diagnostic->type == CosDiagnosticLevel_Error) {
        counter->errors++;
    }
    else {
        counter->warnings++;
    }
}

/**
 * Parses @p input with every strict group set to @p level .
 *
 * @param input The NUL-terminated input.
 * @param level The level to apply to every group, or a negative value to pass
 *   a @c NULL options pointer instead.
 * @param out_counter Receives the diagnostic counts.
 * @param out_object_count Receives the number of objects parsed.
 *
 * @return @c true if the parse ran, @c false if the fixture could not be built.
 */
static bool
parse_at_level_(const char *input,
                int level,
                DiagnosticCounter *out_counter,
                size_t *out_object_count)
{
    CosDoc *doc = NULL;
    CosMemoryStream *stream = NULL;
    CosObjParser *parser = NULL;
    CosDiagnosticHandler *handler = NULL;
    bool result = false;

    out_counter->warnings = 0;
    out_counter->errors = 0;
    *out_object_count = 0;

    doc = cos_doc_create(NULL);
    if (!doc) {
        goto cleanup;
    }

    handler = cos_diagnostic_handler_alloc(&count_diagnostic_, out_counter);
    if (!handler) {
        goto cleanup;
    }
    cos_doc_set_diagnostic_handler(doc, handler);

    stream = cos_memory_stream_create_readonly(input, strlen(input));
    if (!stream) {
        goto cleanup;
    }

    CosParserOptions options = cos_parser_options_make_default();
    for (unsigned int i = 0; i < COS_STRICT_GROUP_COUNT; i++) {
        cos_parser_options_set_strict_level(&options,
                                            (CosStrictGroup)i,
                                            (CosStrictLevel)level);
    }

    parser = cos_obj_parser_create(doc,
                                   (CosStream *)stream,
                                   (level < 0) ? NULL : &options);
    if (!parser) {
        goto cleanup;
    }

    size_t count = 0;
    while (count < MAX_OBJECTS && cos_obj_parser_has_next_object(parser)) {
        CosObjNode * const obj = cos_obj_parser_next_object(parser, NULL);
        if (!obj) {
            break;
        }

        count++;
        cos_obj_node_release(obj);
    }

    *out_object_count = count;
    result = true;

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
    if (handler) {
        cos_diagnostic_handler_free(handler);
    }
    return result;
}

/**
 * Asserts that @p input is silent at Off, warns at Warn, and fails at Error.
 *
 * @param input The NUL-terminated input containing one deviation.
 *
 * @return @c EXIT_SUCCESS if the input behaves as expected at all three levels.
 */
static int
expect_deviation_(const char *input)
{
    DiagnosticCounter counter = {0, 0};
    size_t object_count = 0;

    TEST_EXPECT(parse_at_level_(input, CosStrictLevel_Off, &counter, &object_count));
    TEST_EXPECT(counter.warnings == 0);
    TEST_EXPECT(counter.errors == 0);
    TEST_EXPECT(object_count > 0);

    TEST_EXPECT(parse_at_level_(input, CosStrictLevel_Warn, &counter, &object_count));
    TEST_EXPECT(counter.warnings > 0);
    TEST_EXPECT(counter.errors == 0);
    // Warning must not change the outcome.
    TEST_EXPECT(object_count > 0);

    TEST_EXPECT(parse_at_level_(input, CosStrictLevel_Error, &counter, &object_count));
    TEST_EXPECT(counter.errors > 0);

    return EXIT_SUCCESS;
}

// MARK: - Strict mode tests

static int
strict_refWithDoubleSpace_IsDeviation(void)
{
    return expect_deviation_("[ 1  0 R ]");
}

static int
strict_refWithNewline_IsDeviation(void)
{
    return expect_deviation_("[ 1\n0 R ]");
}

static int
strict_objHeaderWithDoubleSpace_IsDeviation(void)
{
    return expect_deviation_("1  0 obj\nnull\nendobj\n");
}

static int
strict_endObjWithoutEol_IsDeviation(void)
{
    return expect_deviation_("1 0 obj\nnull endobj\n");
}

static int
strict_missingEndObj_IsDeviation(void)
{
    return expect_deviation_("1 0 obj\nnull\n");
}

static int
strict_streamWithBareCr_IsDeviation(void)
{
    // A bare CR after the stream keyword: ISO 32000-1 allows only LF or CRLF.
    return expect_deviation_("1 0 obj\n<< /Length 3 >>\nstream\rabc\nendstream\nendobj\n");
}

static int
strict_streamWithoutEol_IsDeviation(void)
{
    // No end-of-line marker at all after the stream keyword; the space that
    // ends the keyword token becomes the first byte of the 4-byte payload.
    return expect_deviation_("1 0 obj\n<< /Length 4 >>\nstream abc\nendstream\nendobj\n");
}

static int
strict_endStreamWithoutEol_IsDeviation(void)
{
    return expect_deviation_("1 0 obj\n<< /Length 3 >>\nstream\nabcendstream\nendobj\n");
}

/**
 * A negative object number must be rejected at every level, including Off.
 *
 * The numbers are cast to unsigned when the object ID is built, so accepting
 * "-1 0 R" would silently produce object 4294967295.
 */
static int
strict_negativeObjNumber_AlwaysRejected(void)
{
    CosObjNodeType types[MAX_OBJECTS] = {CosObjNodeType_Unknown};

    // A well-formed reference is still an element.
    TEST_EXPECT(parse_array_element_types_("[ 1 0 R ]", types) == 1);
    TEST_EXPECT(types[0] == CosObjNodeType_Reference);

    // A negative object or generation number yields no element at all, rather
    // than a reference to object 4294967295.
    TEST_EXPECT(parse_array_element_types_("[ -1 0 R ]", types) == 0);
    TEST_EXPECT(parse_array_element_types_("[ 1 -1 R ]", types) == 0);

    return EXIT_SUCCESS;
}

/**
 * Well-formed input must not trip any of the new checks.
 */
static int
strict_conformingInput_IsSilent(void)
{
    DiagnosticCounter counter = {0, 0};
    size_t object_count = 0;

    TEST_EXPECT(parse_at_level_("1 0 obj\nnull\nendobj\n",
                                CosStrictLevel_Warn,
                                &counter,
                                &object_count));
    TEST_EXPECT(counter.warnings == 0);
    TEST_EXPECT(counter.errors == 0);

    TEST_EXPECT(parse_at_level_("[ 1 0 R ]",
                                CosStrictLevel_Warn,
                                &counter,
                                &object_count));
    TEST_EXPECT(counter.warnings == 0);
    TEST_EXPECT(counter.errors == 0);

    TEST_EXPECT(parse_at_level_("1 0 obj\n<< /Length 3 >>\nstream\nabc\nendstream\nendobj\n",
                                CosStrictLevel_Warn,
                                &counter,
                                &object_count));
    TEST_EXPECT(counter.warnings == 0);
    TEST_EXPECT(counter.errors == 0);

    return EXIT_SUCCESS;
}

/**
 * A NULL options pointer must behave exactly like the defaults.
 *
 * This is what the call sites that pass NULL are relying on.
 */
static int
strict_nullOptions_MatchesDefaults(void)
{
    DiagnosticCounter null_counter = {0, 0};
    DiagnosticCounter default_counter = {0, 0};
    size_t null_count = 0;
    size_t default_count = 0;

    const char * const inputs[] = {
        "1 0 obj\nnull\nendobj\n",
        "1  0 obj\nnull\nendobj\n",
    };

    for (size_t i = 0; i < (sizeof(inputs) / sizeof(inputs[0])); i++) {
        TEST_EXPECT(parse_at_level_(inputs[i], -1, &null_counter, &null_count));
        TEST_EXPECT(parse_at_level_(inputs[i],
                                    CosStrictLevel_Warn,
                                    &default_counter,
                                    &default_count));

        TEST_EXPECT(null_counter.warnings == default_counter.warnings);
        TEST_EXPECT(null_counter.errors == default_counter.errors);
        TEST_EXPECT(null_count == default_count);
    }

    return EXIT_SUCCESS;
}

static int
strict_defaultOptions_WarnForEveryGroup(void)
{
    const CosParserOptions options = cos_parser_options_make_default();

    for (unsigned int i = 0; i < COS_STRICT_GROUP_COUNT; i++) {
        TEST_EXPECT(cos_parser_options_get_strict_level(&options,
                                                        (CosStrictGroup)i) ==
                    CosStrictLevel_Warn);
    }

    // An out-of-range group must not read past the array.
    TEST_EXPECT(cos_parser_options_get_strict_level(&options,
                                                    (CosStrictGroup)COS_STRICT_GROUP_COUNT) ==
                CosStrictLevel_Off);

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

    /* Context enforcement */
    TEST_EXPECT(parse_realInArray_IsElement() == EXIT_SUCCESS);
    TEST_EXPECT(parse_nullInArray_IsElement() == EXIT_SUCCESS);
    TEST_EXPECT(parse_boolInArray_IsElement() == EXIT_SUCCESS);
    TEST_EXPECT(parse_mixedArray_KeepsOrder() == EXIT_SUCCESS);
    TEST_EXPECT(parse_referenceInArray_IsElement() == EXIT_SUCCESS);
    TEST_EXPECT(parse_bareReference_DoesNotParse() == EXIT_SUCCESS);
    TEST_EXPECT(parse_dictWithNameKey_HasEntry() == EXIT_SUCCESS);
    TEST_EXPECT(parse_dictWithNonNameKey_DropsEntry() == EXIT_SUCCESS);

    /* Flush */
    TEST_EXPECT(flush_DiscardsPeekedObject() == EXIT_SUCCESS);

    /* Strict mode */
    TEST_EXPECT(strict_defaultOptions_WarnForEveryGroup() == EXIT_SUCCESS);
    TEST_EXPECT(strict_conformingInput_IsSilent() == EXIT_SUCCESS);
    TEST_EXPECT(strict_nullOptions_MatchesDefaults() == EXIT_SUCCESS);
    TEST_EXPECT(strict_refWithDoubleSpace_IsDeviation() == EXIT_SUCCESS);
    TEST_EXPECT(strict_refWithNewline_IsDeviation() == EXIT_SUCCESS);
    TEST_EXPECT(strict_objHeaderWithDoubleSpace_IsDeviation() == EXIT_SUCCESS);
    TEST_EXPECT(strict_endObjWithoutEol_IsDeviation() == EXIT_SUCCESS);
    TEST_EXPECT(strict_missingEndObj_IsDeviation() == EXIT_SUCCESS);
    TEST_EXPECT(strict_streamWithBareCr_IsDeviation() == EXIT_SUCCESS);
    TEST_EXPECT(strict_streamWithoutEol_IsDeviation() == EXIT_SUCCESS);
    TEST_EXPECT(strict_endStreamWithoutEol_IsDeviation() == EXIT_SUCCESS);
    TEST_EXPECT(strict_negativeObjNumber_AlwaysRejected() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END
