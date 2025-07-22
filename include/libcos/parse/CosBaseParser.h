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

enum {
    /// The size of the parser's token buffer.
    COS_BASE_PARSER_TOKEN_BUFFER_SIZE = 3,
};

struct CosBaseParser {
    /**
     * The allocator used by the parser.
     */
    CosAllocator *allocator;

    /**
     * The document being parsed.
     */
    CosDoc *doc;

    /**
     * The input stream to read from.
     */
    CosStream *input_stream;
    /**
     * The tokenizer used for parsing.
     */
    CosTokenizer *tokenizer;

    /**
     * The buffer for tokens.
     *
     * The capacity of the buffer is @a token_buffer_size ,
     * and the current number of tokens in the buffer is @a token_count .
     */
    COS_FIELD_SPEC(nonnull, counted_by(token_buffer_size))
    CosToken *token_buffer;

    /**
     * The size or capacity of @a token_buffer .
     */
    size_t token_buffer_size;

    /**
     * The number of tokens currently in @a token_buffer .
     */
    size_t token_count;

    /**
     * The handler for diagnostics.
     */
    CosDiagnosticHandler * COS_Nullable diagnostic_handler;
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
