/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/parse/CosBaseParser.h"

#include "common/Assert.h"
#include "syntax/tokenizer/CosTokenizer.h"

#include "libcos/CosDoc.h"
#include "libcos/common/memory/CosMemory.h"

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

    parser->doc = document;
    parser->input_stream = input_stream;

    parser->tokenizer = tokenizer;

    parser->token_buffer = token_buffer;
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

COS_ASSUME_NONNULL_END
