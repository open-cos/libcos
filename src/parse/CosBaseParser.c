/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/parse/CosBaseParser.h"

#include "common/Assert.h"
#include "syntax/tokenizer/CosTokenizer.h"

#include "libcos/CosDoc.h"
#include "libcos/common/memory/CosMemory.h"

#include <string.h>

COS_ASSUME_NONNULL_BEGIN

bool
cos_base_parser_init(CosBaseParser *parser,
                     CosDoc *document,
                     CosStream *input_stream)
{
    COS_API_PARAM_CHECK(parser != NULL);
    COS_API_PARAM_CHECK(document != NULL);
    COS_API_PARAM_CHECK(input_stream != NULL);
    if (COS_UNLIKELY(!parser || !document || !input_stream)) {
        return false;
    }

    CosAllocator * const allocator = cos_doc_get_allocator(document);
    if (!allocator) {
        return false;
    }

    CosTokenizer *tokenizer = NULL;
    CosToken *token_buffer = NULL;

    tokenizer = cos_tokenizer_create(input_stream);
    if (!tokenizer) {
        goto failure;
    }

    token_buffer = cos_alloc(allocator,
                             sizeof(CosToken) * 3);
    if (!token_buffer) {
        goto failure;
    }

    parser->allocator = allocator;

    parser->doc = document;
    parser->input_stream = input_stream;

    parser->tokenizer = tokenizer;

    parser->token_buffer = token_buffer;
    parser->token_buffer_size = 3; // Initial size of the token buffer.
    parser->token_count = 0;
    parser->diagnostic_handler = cos_doc_get_diagnostic_handler(document);

    return true;

failure:
    if (tokenizer) {
        cos_tokenizer_destroy(tokenizer);
    }
    if (token_buffer) {
        cos_free(allocator, token_buffer);
    }
    return false;
}

void
cos_base_parser_destroy(CosBaseParser *parser)
{
    COS_API_PARAM_CHECK(parser != NULL);
    if (COS_UNLIKELY(!parser)) {
        return;
    }

    // Release all the buffered tokens.
    for (size_t i = 0; i < parser->token_count; i++) {
        CosToken * const token = &(parser->token_buffer[i]);
        cos_token_reset(token);
    }

    cos_tokenizer_destroy(parser->tokenizer);

    cos_free(parser->allocator, parser);
}

CosToken * COS_Nullable
cos_base_parser_get_current_token(CosBaseParser *parser)
{
    COS_API_PARAM_CHECK(parser != NULL);
    if (COS_UNLIKELY(!parser)) {
        return NULL;
    }

    return cos_base_parser_peek_next_token(parser, 0);
}

bool
cos_base_parser_has_next_token(CosBaseParser *parser)
{
    COS_API_PARAM_CHECK(parser != NULL);
    if (COS_UNLIKELY(!parser)) {
        return false;
    }

    const CosToken *token = cos_base_parser_peek_next_token(parser, 0);
    return (token != NULL);
}

CosToken *
cos_base_parser_peek_next_token(CosBaseParser *parser,
                                unsigned int lookahead)
{
    COS_API_PARAM_CHECK(parser != NULL);
    if (COS_UNLIKELY(!parser)) {
        return NULL;
    }

    if (lookahead >= parser->token_buffer_size) {
        return NULL;
    }

    // Fill up the buffer up to the requested lookahead index.
    for (unsigned int i = 0; i <= lookahead; i++) {
        if (i >= parser->token_count) {
            CosToken *token = &(parser->token_buffer[i]);

            if (!cos_tokenizer_get_next_token(parser->tokenizer,
                                              token,
                                              NULL)) {
                return NULL;
            }

            parser->token_count++;
        }
    }

    CosToken * const peeked_token = &parser->token_buffer[lookahead];
    COS_ASSERT(peeked_token != NULL, "Expected a token");
    return peeked_token;
}

void
cos_base_parser_advance(CosBaseParser *parser)
{
    COS_API_PARAM_CHECK(parser != NULL);
    if (COS_UNLIKELY(!parser)) {
        return;
    }

    if (parser->token_count == 0) {
        return;
    }

    // Release the current token.
    CosToken *current_token = &(parser->token_buffer[0]);
    cos_token_reset(current_token);

    const size_t token_buffer_size = parser->token_buffer_size;

    // "Pop" the first token by moving the rest of the buffer left with memmove.
    memmove(&parser->token_buffer[0],
            &parser->token_buffer[1],
            sizeof(parser->token_buffer[0]) * (token_buffer_size - 1));

    // Zero out the last token after the shift.
    memset(&(parser->token_buffer[token_buffer_size - 1]),
           0,
           sizeof(CosToken));

    // Decrement the token count.
    parser->token_count--;
}

bool
cos_base_parser_matches_next_token(CosBaseParser *parser,
                                   CosToken_Type type,
                                   CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(parser != NULL);
    if (COS_UNLIKELY(!parser)) {
        return false;
    }

    CosToken * const token = cos_base_parser_peek_next_token(parser, 0);
    if (!token) {
        return false;
    }

    (void)out_error;

    return (token->type == type);
}

COS_ASSUME_NONNULL_END
