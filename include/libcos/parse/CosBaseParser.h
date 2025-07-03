/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_PARSE_COS_BASE_PARSER_H
#define LIBCOS_PARSE_COS_BASE_PARSER_H

#include "common/CosAPI.h"
#include "syntax/tokenizer/CosToken.h"

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosBaseParser {
    /**
     * The allocator used by the parser.
     */
    CosAllocator *allocator;

    CosDoc *doc;             ///< The document being parsed.
    CosStream *input_stream; ///< The input stream to read from.
    CosTokenizer *tokenizer; ///< The tokenizer used for parsing.

    CosToken *token_buffer;   ///< Buffer for tokens.
    size_t token_buffer_size; ///< Size of the token buffer.
    size_t token_count;       ///< Number of tokens in the buffer.

    CosDiagnosticHandler * COS_Nullable diagnostic_handler; ///< Handler for diagnostics.
};

COS_API bool
cos_base_parser_init(CosBaseParser *parser,
                     CosDoc *document,
                     CosStream *input_stream)
    COS_OWNERSHIP_HOLDS(3)
    COS_WARN_UNUSED_RESULT;

COS_API void
cos_base_parser_destroy(CosBaseParser *parser);

/**
 * @brief Gets the parser's current token.
 *
 * @param parser The parser.
 *
 * @return The current token, or @c NULL if there is no current token.
 */
COS_API CosToken * COS_Nullable
cos_base_parser_get_current_token(CosBaseParser *parser);

/**
 * @brief Checks if there is a next token.
 *
 * @param parser The parser.
 *
 * @return @c true if there is a next token, otherwise @c false.
 */
COS_API bool
cos_base_parser_has_next_token(CosBaseParser *parser);

/**
 * @brief Peeks a next token without consuming it.
 *
 * Additional tokens are read to fill the buffer up to the specified lookahead index.
 *
 * @param parser The parser.
 * @param lookahead The lookahead index.
 *
 * @return The peeked next token, or @c NULL if there is no next token.
 */
COS_API CosToken * COS_Nullable
cos_base_parser_peek_next_token(CosBaseParser *parser,
                                unsigned int lookahead);

/**
 * @brief Advances the parser to the next token.
 *
 * This function advances the parser to the next token by shifting the token buffer,
 * releasing the current token. Additional tokens are *not* read to re-fill the buffer.
 *
 * @param parser The parser to be advanced.
 */
COS_API void
cos_base_parser_advance(CosBaseParser *parser);

/**
 * @brief Checks if the next token matches the specified type.
 *
 * @param parser The parser to check.
 * @param type The token type to match.
 * @param out_error Optional output parameter for errors.
 *
 * @return @c true if the next token matches the specified type, otherwise @c false.
 */
COS_API bool
cos_base_parser_matches_next_token(CosBaseParser *parser,
                                   CosToken_Type type,
                                   CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_PARSE_COS_BASE_PARSER_H */
