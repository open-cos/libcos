/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_XREF_COS_XREF_TABLE_PARSER_H
#define LIBCOS_XREF_COS_XREF_TABLE_PARSER_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief Deallocates a cross-reference table parser.
 *
 * @param parser The parser to deallocate.
 */
void
cos_xref_table_parser_destroy(CosXrefTableParser *parser)
    COS_DEALLOCATOR_FUNC;

/**
 * @brief Creates a new cross-reference table parser.
 *
 * The parser borrows the tokenizer; the caller retains ownership.
 *
 * @param document The document being parsed.
 * @param tokenizer The tokenizer to use for parsing.
 *
 * @return A new parser, or @c NULL if an error occurred.
 */
CosXrefTableParser * COS_Nullable
cos_xref_table_parser_create(CosDoc *document,
                             CosTokenizer *tokenizer)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_xref_table_parser_destroy);

/**
 * @brief Parses a cross-reference table from the current position in the tokenizer.
 *
 * On entry, the tokenizer should be positioned such that the next token is
 * the @c xref keyword.
 *
 * @param parser The parser.
 * @param out_error On input, a pointer to an error object, or @c NULL.
 *
 * @return The parsed cross-reference table, or @c NULL if an error occurred.
 */
CosXrefTable * COS_Nullable
cos_xref_table_parser_parse(CosXrefTableParser *parser,
                            CosError * COS_Nullable out_error)
    COS_OWNERSHIP_RETURNS;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_XREF_COS_XREF_TABLE_PARSER_H */
