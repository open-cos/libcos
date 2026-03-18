/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/xref/CosXrefTableParser.h"

#include "common/Assert.h"
#include "parse/CosBaseParser.h"
#include "syntax/tokenizer/CosToken.h"
#include "syntax/tokenizer/CosTokenValue.h"

#include "libcos/common/CosArray.h"
#include "libcos/common/CosError.h"
#include "libcos/common/CosMacros.h"
#include "libcos/xref/table/CosXrefEntry.h"
#include "libcos/xref/table/CosXrefSection.h"
#include "libcos/xref/table/CosXrefSubsection.h"
#include "libcos/xref/table/CosXrefTable.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosXrefTableParser {
    CosBaseParser base;
};

static void
cos_xref_entry_release_callback_(void *item);

static CosXrefSection * COS_Nullable
cos_xref_table_parser_parse_section_(CosXrefTableParser *parser,
                                     CosError * COS_Nullable out_error);

static CosXrefSubsection * COS_Nullable
cos_xref_table_parser_parse_subsection_(CosXrefTableParser *parser,
                                        CosError * COS_Nullable out_error);

static bool
cos_xref_table_parser_read_subsection_header_(CosXrefTableParser *parser,
                                              unsigned int *out_first_object_number,
                                              unsigned int *out_entry_count,
                                              CosError * COS_Nullable out_error);

static bool
cos_xref_table_parser_read_entry_(CosXrefTableParser *parser,
                                  CosXrefEntry *out_entry,
                                  CosError * COS_Nullable out_error);

CosXrefTableParser *
cos_xref_table_parser_create(CosDoc *document,
                             CosTokenizer *tokenizer)
{
    COS_API_PARAM_CHECK(document != NULL);
    COS_API_PARAM_CHECK(tokenizer != NULL);
    if (COS_UNLIKELY(!document || !tokenizer)) {
        return NULL;
    }

    CosXrefTableParser *parser = NULL;

    parser = calloc(1, sizeof(CosXrefTableParser));
    if (COS_UNLIKELY(!parser)) {
        goto failure;
    }

    if (!cos_base_parser_init_with_tokenizer(&(parser->base),
                                             document,
                                             tokenizer)) {
        goto failure;
    }

    return parser;

failure:
    if (parser) {
        free(parser);
    }
    return NULL;
}

void
cos_xref_table_parser_destroy(CosXrefTableParser *parser)
{
    if (COS_UNLIKELY(!parser)) {
        return;
    }

    cos_base_parser_destroy(&(parser->base));
}

CosXrefTable *
cos_xref_table_parser_parse(CosXrefTableParser *parser,
                            CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(parser != NULL);
    if (COS_UNLIKELY(!parser)) {
        return NULL;
    }

    CosXrefTable *table = NULL;

    // Expect the 'xref' keyword.
    if (!cos_base_parser_matches_next_token(&(parser->base),
                                            CosToken_Type_XRef,
                                            out_error)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                           "Expected 'xref' keyword"),
                            out_error);
        goto failure;
    }
    cos_base_parser_advance(&(parser->base));

    table = cos_xref_table_create();
    if (COS_UNLIKELY(!table)) {
        goto failure;
    }

    // Parse sections until the 'trailer' keyword or EOF.
    while (!cos_base_parser_matches_next_token(&(parser->base),
                                               CosToken_Type_Trailer,
                                               out_error) &&
           cos_base_parser_has_next_token(&(parser->base))) {
        CosXrefSection * const section = cos_xref_table_parser_parse_section_(parser,
                                                                              out_error);
        if (!section) {
            goto failure;
        }

        if (!cos_xref_table_add_section(table, section, out_error)) {
            cos_xref_section_destroy(section);
            goto failure;
        }
    }

    return table;

failure:
    if (table) {
        cos_xref_table_destroy(table);
    }
    return NULL;
}

// MARK: - Implementation

static void
cos_xref_entry_release_callback_(void *item)
{
    COS_IMPL_PARAM_CHECK(item != NULL);
    if (COS_UNLIKELY(!item)) {
        return;
    }

    CosXrefEntry * const entry = *(CosXrefEntry **)item;
    free(entry);
}

static const CosArrayCallbacks cos_xref_entry_array_callbacks_ = {
    .release = cos_xref_entry_release_callback_,
};

static CosXrefSection *
cos_xref_table_parser_parse_section_(CosXrefTableParser *parser,
                                     CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    if (COS_UNLIKELY(!parser)) {
        return NULL;
    }

    CosXrefSection *section = NULL;

    section = cos_xref_section_create();
    if (COS_UNLIKELY(!section)) {
        goto failure;
    }

    // Parse subsections until the 'trailer' keyword or EOF.
    while (!cos_base_parser_matches_next_token(&(parser->base),
                                               CosToken_Type_Trailer,
                                               out_error) &&
           !cos_base_parser_matches_next_token(&(parser->base),
                                               CosToken_Type_EOF,
                                               out_error) &&
           cos_base_parser_has_next_token(&(parser->base))) {
        // A subsection header starts with an Integer token.
        if (!cos_base_parser_matches_next_token(&(parser->base),
                                                CosToken_Type_Integer,
                                                out_error)) {
            break;
        }

        CosXrefSubsection * const subsection = cos_xref_table_parser_parse_subsection_(parser,
                                                                                       out_error);
        if (!subsection) {
            goto failure;
        }

        if (!cos_xref_section_add_subsection(section, subsection, out_error)) {
            cos_xref_subsection_destroy(subsection);
            goto failure;
        }
    }

    return section;

failure:
    if (section) {
        cos_xref_section_destroy(section);
    }
    return NULL;
}

static CosXrefSubsection *
cos_xref_table_parser_parse_subsection_(CosXrefTableParser *parser,
                                        CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    if (COS_UNLIKELY(!parser)) {
        return NULL;
    }

    CosArray *entries = NULL;
    CosXrefEntry *entry = NULL;
    CosXrefSubsection *subsection = NULL;

    unsigned int first_object_number = 0;
    unsigned int entry_count = 0;

    if (!cos_xref_table_parser_read_subsection_header_(parser,
                                                       &first_object_number,
                                                       &entry_count,
                                                       out_error)) {
        goto failure;
    }

    // The entries array uses a release callback so that cos_array_destroy
    // frees each CosXrefEntry on cleanup.
    entries = cos_array_create(sizeof(CosXrefEntry *),
                               &cos_xref_entry_array_callbacks_,
                               entry_count);
    if (COS_UNLIKELY(!entries)) {
        goto failure;
    }

    for (unsigned int i = 0; i < entry_count; i++) {
        entry = calloc(1, sizeof(CosXrefEntry));
        if (COS_UNLIKELY(!entry)) {
            goto failure;
        }

        if (!cos_xref_table_parser_read_entry_(parser, entry, out_error)) {
            goto failure;
        }

        if (!cos_array_append_item(entries, &entry, out_error)) {
            goto failure;
        }

        // Ownership of entry was copied into the array; clear the local pointer
        // so the failure path does not double-free it.
        entry = NULL;
    }

    // cos_xref_subsection_create takes ownership of the entries array.
    subsection = cos_xref_subsection_create(first_object_number,
                                            entry_count,
                                            entries);
    entries = NULL; // ownership transferred (freed by subsection_create on failure too)
    if (COS_UNLIKELY(!subsection)) {
        goto failure;
    }

    return subsection;

failure:
    if (entry) {
        free(entry);
    }
    if (entries) {
        cos_array_destroy(entries);
    }
    return NULL;
}

static bool
cos_xref_table_parser_read_subsection_header_(CosXrefTableParser *parser,
                                              unsigned int *out_first_object_number,
                                              unsigned int *out_entry_count,
                                              CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(out_first_object_number != NULL);
    COS_IMPL_PARAM_CHECK(out_entry_count != NULL);
    if (COS_UNLIKELY(!parser || !out_first_object_number || !out_entry_count)) {
        return false;
    }

    // Read the first object number.
    const CosToken * const first_token = cos_base_parser_get_current_token(&(parser->base));
    if (COS_UNLIKELY(!first_token || first_token->type != CosToken_Type_Integer)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                           "Expected integer for subsection first object number"),
                            out_error);
        goto failure;
    }

    int first_obj_num = 0;
    if (!cos_token_value_get_integer_number(&first_token->value, &first_obj_num)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                           "Invalid subsection first object number"),
                            out_error);
        goto failure;
    }
    cos_base_parser_advance(&(parser->base));

    // Read the entry count.
    const CosToken * const count_token = cos_base_parser_get_current_token(&(parser->base));
    if (COS_UNLIKELY(!count_token || count_token->type != CosToken_Type_Integer)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                           "Expected integer for subsection entry count"),
                            out_error);
        goto failure;
    }

    int entry_count = 0;
    if (!cos_token_value_get_integer_number(&count_token->value, &entry_count) ||
        entry_count < 0) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                           "Invalid subsection entry count"),
                            out_error);
        goto failure;
    }
    cos_base_parser_advance(&(parser->base));

    *out_first_object_number = (unsigned int)first_obj_num;
    *out_entry_count = (unsigned int)entry_count;

    return true;

failure:
    return false;
}

static bool
cos_xref_table_parser_read_entry_(CosXrefTableParser *parser,
                                  CosXrefEntry *out_entry,
                                  CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(out_entry != NULL);
    if (COS_UNLIKELY(!parser || !out_entry)) {
        return false;
    }

    // Read the first number (byte offset or next free object number).
    const CosToken * const first_token = cos_base_parser_get_current_token(&(parser->base));
    if (COS_UNLIKELY(!first_token || first_token->type != CosToken_Type_Integer)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                           "Expected integer for xref entry first field"),
                            out_error);
        goto failure;
    }

    int first_number = 0;
    if (!cos_token_value_get_integer_number(&first_token->value, &first_number)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                           "Invalid xref entry first field"),
                            out_error);
        goto failure;
    }
    cos_base_parser_advance(&(parser->base));

    // Read the generation number.
    const CosToken * const gen_token = cos_base_parser_get_current_token(&(parser->base));
    if (COS_UNLIKELY(!gen_token || gen_token->type != CosToken_Type_Integer)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                           "Expected integer for xref entry generation number"),
                            out_error);
        goto failure;
    }

    int gen_number = 0;
    if (!cos_token_value_get_integer_number(&gen_token->value, &gen_number)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                           "Invalid xref entry generation number"),
                            out_error);
        goto failure;
    }
    cos_base_parser_advance(&(parser->base));

    // Read the keyword: 'n' (in-use) or 'f' (free).
    const CosToken * const keyword_token = cos_base_parser_get_current_token(&(parser->base));
    if (COS_UNLIKELY(!keyword_token)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                           "Missing xref entry keyword"),
                            out_error);
        goto failure;
    }

    if (keyword_token->type == CosToken_Type_N) {
        cos_xref_entry_init_in_use(out_entry,
                                   (unsigned int)first_number,
                                   (unsigned int)gen_number);
    }
    else if (keyword_token->type == CosToken_Type_F) {
        cos_xref_entry_init_free(out_entry,
                                 (unsigned int)first_number,
                                 (unsigned int)gen_number);
    }
    else {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                           "Invalid xref entry keyword"),
                            out_error);
        goto failure;
    }

    cos_base_parser_advance(&(parser->base));

    return true;

failure:
    return false;
}

COS_ASSUME_NONNULL_END
