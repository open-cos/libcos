//
// Created by david on 03/06/23.
//

#ifndef LIBCOS_COS_TOKENIZER_H
#define LIBCOS_COS_TOKENIZER_H

#include <libcos/common/CosDefines.h>
#include <libcos/io/CosInputStream.h>

struct CosTokenizer;
typedef struct CosTokenizer CosTokenizer;

void
cos_token_free(struct CosToken *token);

CosTokenizer *
cos_tokenizer_alloc(CosInputStream *input_stream);

void
cos_tokenizer_free(CosTokenizer *tokenizer);

/**
 * Get the input stream associated with the tokenizer.
 *
 * @param tokenizer The tokenizer.
 *
 * @return The input stream associated with the tokenizer.
 */
CosInputStream *
cos_tokenizer_get_input_stream(const CosTokenizer *tokenizer);

/**
 * Peek the next token without consuming it.
 *
 * @param tokenizer The tokenizer.
 *
 * @return The next token.
 */
struct CosToken *
cos_tokenizer_peek_token(CosTokenizer *tokenizer);

/**
 * Consume the next token.
 *
 * @param tokenizer The tokenizer.
 *
 * @return The next token.
 */
CosToken *
cos_tokenizer_next_token(CosTokenizer *tokenizer);

#endif /* LIBCOS_COS_TOKENIZER_H */
