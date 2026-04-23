/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "CosTest.h"

#include <libcos/CosDoc.h>
#include <libcos/common/CosError.h>
#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>
#include <libcos/syntax/tokenizer/CosTokenizer.h>
#include <libcos/xref/CosXrefTableParser.h>
#include <libcos/xref/table/CosXrefEntry.h>
#include <libcos/xref/table/CosXrefSection.h>
#include <libcos/xref/table/CosXrefSubsection.h>
#include <libcos/xref/table/CosXrefTable.h>

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

/**
 * @brief Parses an xref table from an in-memory string.
 *
 * Creates a CosDoc, wraps the input in a CosMemoryStream, creates a tokenizer
 * and parser, runs the parse, then tears everything down (except the returned
 * table which the caller owns).
 *
 * @param input The NUL-terminated input string.
 * @param out_error On input, a pointer to an error object, or @c NULL.
 *
 * @return The parsed table, or @c NULL on failure.
 */
static CosXrefTable * COS_Nullable
parse_xref_from_string_(const char *input,
                        CosError * COS_Nullable out_error)
{
    CosDoc *doc = NULL;
    CosMemoryStream *stream = NULL;
    CosTokenizer *tokenizer = NULL;
    CosXrefTableParser *parser = NULL;
    CosXrefTable *table = NULL;
    CosXrefSection *section = NULL;

    doc = cos_doc_create(NULL);
    if (!doc) {
        goto cleanup;
    }

    stream = cos_memory_stream_create((void *)(uintptr_t)input,
                                      strlen(input),
                                      false);
    if (!stream) {
        goto cleanup;
    }

    tokenizer = cos_tokenizer_create((CosStream *)stream);
    if (!tokenizer) {
        goto cleanup;
    }

    parser = cos_xref_table_parser_create(doc, tokenizer);
    if (!parser) {
        goto cleanup;
    }

    section = cos_xref_table_parser_parse_section(parser, out_error);
    if (!section) {
        goto cleanup;
    }

    table = cos_xref_table_create();
    if (!table) {
        cos_xref_section_destroy(section);
        section = NULL;
        goto cleanup;
    }

    if (!cos_xref_table_add_section(table, section, out_error)) {
        cos_xref_section_destroy(section);
        cos_xref_table_destroy(table);
        table = NULL;
        goto cleanup;
    }
    section = NULL; // ownership transferred to table

cleanup:
    if (parser) {
        cos_xref_table_parser_destroy(parser);
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
    return table;
}

// MARK: - Happy-path structural tests

static int
parse_minimalFreeEntry_Succeeds(void)
{
    const char *input =
        "xref\n"
        "0 1\n"
        "0000000000 65535 f \n"
        "trailer\n";
    CosError error = cos_error_none();
    CosXrefTable * const table = parse_xref_from_string_(input, &error);

    TEST_EXPECT(table != NULL);
    TEST_EXPECT(cos_xref_table_get_section_count(table) == 1);

    CosXrefSection * const section = cos_xref_table_get_section(table, 0, NULL);
    TEST_EXPECT(section != NULL);
    TEST_EXPECT(cos_xref_section_get_subsection_count(section) == 1);

    CosXrefSubsection * const subsection = cos_xref_section_get_subsection(section, 0, NULL);
    TEST_EXPECT(subsection != NULL);
    TEST_EXPECT(cos_xref_subsection_get_first_object_number(subsection) == 0);
    TEST_EXPECT(cos_xref_subsection_get_entry_count(subsection) == 1);

    const CosXrefEntry * const entry = cos_xref_subsection_get_entry(subsection, 0, NULL);
    TEST_EXPECT(entry != NULL);
    TEST_EXPECT(entry->type == CosXrefEntryType_Free);
    TEST_EXPECT(entry->value.free.gen_number == 65535);

    cos_xref_table_destroy(table);
    return EXIT_SUCCESS;
}

static int
parse_singleInUseEntry_CorrectValues(void)
{
    const char *input =
        "xref\n"
        "1 1\n"
        "0000000100 00000 n \n"
        "trailer\n";
    CosError error = cos_error_none();
    CosXrefTable * const table = parse_xref_from_string_(input, &error);

    TEST_EXPECT(table != NULL);

    CosXrefSection * const section = cos_xref_table_get_section(table, 0, NULL);
    TEST_EXPECT(section != NULL);

    CosXrefSubsection * const subsection = cos_xref_section_get_subsection(section, 0, NULL);
    TEST_EXPECT(subsection != NULL);
    TEST_EXPECT(cos_xref_subsection_get_entry_count(subsection) == 1);

    const CosXrefEntry * const entry = cos_xref_subsection_get_entry(subsection, 0, NULL);
    TEST_EXPECT(entry != NULL);
    TEST_EXPECT(entry->type == CosXrefEntryType_InUse);
    TEST_EXPECT(entry->value.in_use.byte_offset == 100);
    TEST_EXPECT(entry->value.in_use.gen_number == 0);

    cos_xref_table_destroy(table);
    return EXIT_SUCCESS;
}

static int
parse_multipleEntries_CorrectCount(void)
{
    /* One subsection with obj 0 (free) + objs 1-3 (in-use), range "0 4". */
    const char *input =
        "xref\n"
        "0 4\n"
        "0000000000 65535 f \n"
        "0000000100 00000 n \n"
        "0000000200 00000 n \n"
        "0000000300 00000 n \n"
        "trailer\n";
    CosError error = cos_error_none();
    CosXrefTable * const table = parse_xref_from_string_(input, &error);

    TEST_EXPECT(table != NULL);
    TEST_EXPECT(cos_xref_table_get_section_count(table) == 1);

    CosXrefSection * const section = cos_xref_table_get_section(table, 0, NULL);
    TEST_EXPECT(section != NULL);
    TEST_EXPECT(cos_xref_section_get_subsection_count(section) == 1);

    CosXrefSubsection * const subsection = cos_xref_section_get_subsection(section, 0, NULL);
    TEST_EXPECT(subsection != NULL);
    TEST_EXPECT(cos_xref_subsection_get_entry_count(subsection) == 4);

    cos_xref_table_destroy(table);
    return EXIT_SUCCESS;
}

static int
parse_twoSubsections_CorrectStructure(void)
{
    const char *input =
        "xref\n"
        "0 1\n"
        "0000000000 65535 f \n"
        "5 2\n"
        "0000000100 00000 n \n"
        "0000000200 00000 n \n"
        "trailer\n";
    CosError error = cos_error_none();
    CosXrefTable * const table = parse_xref_from_string_(input, &error);

    TEST_EXPECT(table != NULL);
    TEST_EXPECT(cos_xref_table_get_section_count(table) == 1);

    CosXrefSection * const section = cos_xref_table_get_section(table, 0, NULL);
    TEST_EXPECT(section != NULL);
    TEST_EXPECT(cos_xref_section_get_subsection_count(section) == 2);

    CosXrefSubsection * const first = cos_xref_section_get_subsection(section, 0, NULL);
    TEST_EXPECT(first != NULL);
    TEST_EXPECT(cos_xref_subsection_get_first_object_number(first) == 0);

    CosXrefSubsection * const second = cos_xref_section_get_subsection(section, 1, NULL);
    TEST_EXPECT(second != NULL);
    TEST_EXPECT(cos_xref_subsection_get_first_object_number(second) == 5);

    cos_xref_table_destroy(table);
    return EXIT_SUCCESS;
}

static int
parse_emptyXref_ReturnsSectionWithNoSubsections(void)
{
    const char *input =
        "xref\n"
        "trailer\n";
    CosError error = cos_error_none();
    CosXrefTable * const table = parse_xref_from_string_(input, &error);

    TEST_EXPECT(table != NULL);
    TEST_EXPECT(cos_xref_table_get_section_count(table) == 1);

    CosXrefSection * const section = cos_xref_table_get_section(table, 0, NULL);
    TEST_EXPECT(section != NULL);
    TEST_EXPECT(cos_xref_section_get_subsection_count(section) == 0);

    cos_xref_table_destroy(table);
    return EXIT_SUCCESS;
}

// MARK: - Lookup tests

static int
findEntry_existingInUseObject_ReturnsEntry(void)
{
    const char *input =
        "xref\n"
        "0 2\n"
        "0000000000 65535 f \n"
        "0000000100 00000 n \n"
        "trailer\n";
    CosXrefTable * const table = parse_xref_from_string_(input, NULL);
    TEST_EXPECT(table != NULL);

    const CosXrefEntry * const entry =
        cos_xref_table_find_entry_for_obj_num(table, 1, NULL);
    TEST_EXPECT(entry != NULL);
    TEST_EXPECT(entry->type == CosXrefEntryType_InUse);
    TEST_EXPECT(entry->value.in_use.byte_offset == 100);

    cos_xref_table_destroy(table);
    return EXIT_SUCCESS;
}

static int
findEntry_freeObjectZero_ReturnsFreeEntry(void)
{
    const char *input =
        "xref\n"
        "0 1\n"
        "0000000000 65535 f \n"
        "trailer\n";
    CosXrefTable * const table = parse_xref_from_string_(input, NULL);
    TEST_EXPECT(table != NULL);

    const CosXrefEntry * const entry =
        cos_xref_table_find_entry_for_obj_num(table, 0, NULL);
    TEST_EXPECT(entry != NULL);
    TEST_EXPECT(entry->type == CosXrefEntryType_Free);

    cos_xref_table_destroy(table);
    return EXIT_SUCCESS;
}

static int
findEntry_objectNotInTable_ReturnsNull(void)
{
    const char *input =
        "xref\n"
        "0 3\n"
        "0000000000 65535 f \n"
        "0000000100 00000 n \n"
        "0000000200 00000 n \n"
        "trailer\n";
    CosXrefTable * const table = parse_xref_from_string_(input, NULL);
    TEST_EXPECT(table != NULL);

    const CosXrefEntry * const entry =
        cos_xref_table_find_entry_for_obj_num(table, 99, NULL);
    TEST_EXPECT(entry == NULL);

    cos_xref_table_destroy(table);
    return EXIT_SUCCESS;
}

static int
findEntry_objectInSecondSubsection_ReturnsEntry(void)
{
    const char *input =
        "xref\n"
        "0 1\n"
        "0000000000 65535 f \n"
        "5 2\n"
        "0000000100 00000 n \n"
        "0000000200 00000 n \n"
        "trailer\n";
    CosXrefTable * const table = parse_xref_from_string_(input, NULL);
    TEST_EXPECT(table != NULL);

    /* Object 6 is the second entry in the second subsection (first=5). */
    const CosXrefEntry * const entry =
        cos_xref_table_find_entry_for_obj_num(table, 6, NULL);
    TEST_EXPECT(entry != NULL);
    TEST_EXPECT(entry->type == CosXrefEntryType_InUse);
    TEST_EXPECT(entry->value.in_use.byte_offset == 200);

    cos_xref_table_destroy(table);
    return EXIT_SUCCESS;
}

// MARK: - Error / malformed-input tests

static int
parse_missingXrefKeyword_ReturnsNull(void)
{
    /* Starts with an integer rather than the 'xref' keyword. */
    const char *input =
        "1 0\n"
        "0000000009 00000 n \n"
        "trailer\n";
    CosError error = cos_error_none();
    CosXrefTable * const table = parse_xref_from_string_(input, &error);

    TEST_EXPECT(table == NULL);
    TEST_EXPECT(error.code == COS_ERROR_XREF);

    return EXIT_SUCCESS;
}

static int
parse_invalidEntryKeyword_ReturnsNull(void)
{
    /* Entry keyword is 'x' instead of 'n' or 'f'. */
    const char *input =
        "xref\n"
        "0 1\n"
        "0000000000 65535 x \n"
        "trailer\n";
    CosError error = cos_error_none();
    CosXrefTable * const table = parse_xref_from_string_(input, &error);

    TEST_EXPECT(table == NULL);

    return EXIT_SUCCESS;
}

static int
parse_truncatedSubsectionHeader_ReturnsNull(void)
{
    /* Subsection header has only one integer instead of two. */
    const char *input =
        "xref\n"
        "0\n"
        "trailer\n";
    CosError error = cos_error_none();
    CosXrefTable * const table = parse_xref_from_string_(input, &error);

    TEST_EXPECT(table == NULL);

    return EXIT_SUCCESS;
}

static int
parse_truncatedEntry_MissingKeyword_ReturnsNull(void)
{
    /* Entry is missing the 'n'/'f' keyword - only two integers present. */
    const char *input =
        "xref\n"
        "0 1\n"
        "0000000000 65535\n"
        "trailer\n";
    CosError error = cos_error_none();
    CosXrefTable * const table = parse_xref_from_string_(input, &error);

    TEST_EXPECT(table == NULL);

    return EXIT_SUCCESS;
}

// MARK: - Test driver

TEST_MAIN()
{
    /* Happy-path structural tests */
    TEST_EXPECT(parse_minimalFreeEntry_Succeeds() == EXIT_SUCCESS);
    TEST_EXPECT(parse_singleInUseEntry_CorrectValues() == EXIT_SUCCESS);
    TEST_EXPECT(parse_multipleEntries_CorrectCount() == EXIT_SUCCESS);
    TEST_EXPECT(parse_twoSubsections_CorrectStructure() == EXIT_SUCCESS);
    TEST_EXPECT(parse_emptyXref_ReturnsSectionWithNoSubsections() == EXIT_SUCCESS);

    /* Lookup tests */
    TEST_EXPECT(findEntry_existingInUseObject_ReturnsEntry() == EXIT_SUCCESS);
    TEST_EXPECT(findEntry_freeObjectZero_ReturnsFreeEntry() == EXIT_SUCCESS);
    TEST_EXPECT(findEntry_objectNotInTable_ReturnsNull() == EXIT_SUCCESS);
    TEST_EXPECT(findEntry_objectInSecondSubsection_ReturnsEntry() == EXIT_SUCCESS);

    /* Error / malformed-input tests */
    TEST_EXPECT(parse_missingXrefKeyword_ReturnsNull() == EXIT_SUCCESS);
    TEST_EXPECT(parse_invalidEntryKeyword_ReturnsNull() == EXIT_SUCCESS);
    TEST_EXPECT(parse_truncatedSubsectionHeader_ReturnsNull() == EXIT_SUCCESS);
    TEST_EXPECT(parse_truncatedEntry_MissingKeyword_ReturnsNull() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END
