/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/xref/CosXrefTableParser.h"

#include "common/Assert.h"
#include "common/CharacterSet.h"
#include "io/CosStream.h"

#include "libcos/common/CosError.h"
#include "libcos/common/CosMacros.h"
#include "libcos/common/CosScanner.h"

#include <libcos/xref/CosXrefEntry.h>
#include <libcos/xref/CosXrefSection.h>
#include <libcos/xref/CosXrefSubsection.h>
#include <libcos/xref/CosXrefTable.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

#define COS_XREF_TABLE_ENTRY_SIZE 20
#define COS_XREF_TABLE_ENTRY_FIRST_NUMBER_SIZE 10
#define COS_XREF_TABLE_ENTRY_SECOND_NUMBER_SIZE 5

typedef struct CosXrefEntryItem {
    size_t required_length;
    unsigned long number;
} CosXrefEntryItem;

struct CosXrefTableParser {
    CosStream *input_stream;

    CosScanner *scanner;

    bool strict;
};

static CosXrefSection * COS_Nullable
cos_xref_table_parser_parse_section_(CosXrefTableParser *parser,
                                     CosError * COS_Nullable out_error);

static CosXrefSubsection * COS_Nullable
cos_xref_table_parser_parse_subsection_(CosXrefTableParser *parser);

static bool
cos_xref_table_parser_read_subsection_header_(CosXrefTableParser *parser,
                                              unsigned int *first_object_number,
                                              unsigned int *entry_count);

static bool
cos_xref_table_parser_read_entry_(CosXrefTableParser *parser,
                                  CosXrefEntry *entry,
                                  CosError * COS_Nullable error);

static bool
cos_read_entry_item_(CosXrefTableParser *parser,
                     CosXrefEntryItem *item,
                     CosError * COS_Nullable error);

CosXrefTableParser *
cos_xref_table_parser_alloc(void)
{
    CosXrefTableParser *parser = NULL;

    parser = calloc(1, sizeof(CosXrefTableParser));
    if (!parser) {
        return NULL;
    }

    return parser;
}

void
cos_xref_table_parser_free(CosXrefTableParser *parser)
{
    if (!parser) {
        return;
    }

    free(parser);
}

static CosXrefSection *
cos_xref_table_parser_parse_section_(CosXrefTableParser *parser,
                                     CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    CosXrefSection *section = cos_xref_section_alloc();
    if (!section) {
        return NULL;
    }

    CosXrefSubsection *subsection = cos_xref_table_parser_parse_subsection_(parser);
    if (subsection && !cos_xref_section_add_subsection(section, subsection, out_error)) {
        goto failure;
    }

    return section;

failure:
    if (section) {
        cos_xref_section_free(section);
    }
    return NULL;
}

static CosXrefSubsection *
cos_xref_table_parser_parse_subsection_(CosXrefTableParser *parser)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    unsigned int first_object_number;
    unsigned int entry_count;

    if (!cos_xref_table_parser_read_subsection_header_(parser,
                                                       &first_object_number,
                                                       &entry_count)) {
        return NULL;
    }

    // Create a new subsection and read the entries.
    CosXrefSubsection *subsection = cos_xref_subsection_alloc(first_object_number,
                                                              entry_count);
    if (!subsection) {
        return NULL;
    }

    CosError error = {0};

    for (size_t i = 0; i < entry_count; i++) {
        CosXrefEntry * const entry = &(subsection->entries[i]);

        if (!cos_xref_table_parser_read_entry_(parser, entry, &error)) {
        }
    }

    return subsection;
}

static bool
cos_xref_table_parser_read_subsection_header_(CosXrefTableParser *parser,
                                              unsigned int *first_object_number,
                                              unsigned int *entry_count)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(first_object_number != NULL);
    COS_PARAMETER_ASSERT(entry_count != NULL);
    if (!parser || !first_object_number || !entry_count) {
        return false;
    }

    return false;
}

static bool
cos_xref_table_parser_read_entry_(CosXrefTableParser *parser,
                                  CosXrefEntry *entry,
                                  CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(entry != NULL);
    if (!parser || !entry) {
        return false;
    }

    unsigned char bytes[COS_XREF_TABLE_ENTRY_SIZE] = {0};

    const size_t num_bytes_read = cos_stream_read(parser->input_stream,
                                                  bytes,
                                                  sizeof(bytes),
                                                  error);
    if (num_bytes_read != sizeof(bytes)) {
        return false;
    }

    cos_scanner_set_input(parser->scanner, (char *)bytes, sizeof(bytes));

    CosXrefEntryItem items[2] = {
        {.required_length = COS_XREF_TABLE_ENTRY_FIRST_NUMBER_SIZE, .number = 0},
        {.required_length = COS_XREF_TABLE_ENTRY_SECOND_NUMBER_SIZE, .number = 0},
    };

    for (size_t i = 0; i < COS_ARRAY_SIZE(items); i++) {
        CosXrefEntryItem *item = &(items[i]);
        if (!cos_read_entry_item_(parser, item, error)) {
            return false;
        }
    }

    char entry_keyword = 0;
    if (!cos_scanner_read_char(parser->scanner, &entry_keyword)) {
        cos_error_propagate(error,
                            cos_error_make(COS_ERROR_XREF,
                                           "table entry is missing a keyword"));
        return false;
    }

    switch (entry_keyword) {
        case 'n': {
            cos_xref_entry_init_in_use(entry,
                                       items[0].number,
                                       items[1].number);
        } break;
        case 'f': {
            cos_xref_entry_init_free(entry,
                                     items[0].number,
                                     items[1].number);
        } break;

        default: {
            cos_error_propagate(error,
                                cos_error_make(COS_ERROR_XREF,
                                               "table entry has an invalid keyword"));
            return false;
        }
    }

    return true;
}

static bool
cos_read_entry_item_(CosXrefTableParser *parser,
                     CosXrefEntryItem *item,
                     CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(item != NULL);
    if (!parser || !item) {
        return false;
    }

    // Read the number.
    unsigned long number = 0;
    const size_t scan_count = cos_scanner_read_unsigned(parser->scanner,
                                                        item->required_length,
                                                        &number,
                                                        error);
    if (scan_count != item->required_length) {
        if (scan_count == 0) {
            cos_error_propagate(error,
                                cos_error_make(COS_ERROR_XREF,
                                               "table entry item is missing"));
            return false;
        }
        else {
            if (parser->strict) {
                cos_error_propagate(error,
                                    cos_error_make(COS_ERROR_XREF,
                                                   "table entry item is invalid"));
                return false;
            }
        }
    }

    // Read the space separator.
    if (!cos_scanner_match_char(parser->scanner, CosCharacterSet_Space)) {
        cos_error_propagate(error,
                            cos_error_make(COS_ERROR_XREF,
                                           "table entry is missing a space separator"));
        return false;
    }

    item->number = number;

    return true;
}

COS_ASSUME_NONNULL_END
