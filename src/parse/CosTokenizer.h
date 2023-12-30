//
// Created by david on 03/06/23.
//

#ifndef LIBCOS_COS_TOKENIZER_H
#define LIBCOS_COS_TOKENIZER_H

#include "parse/CosToken.h"

#include <libcos/common/CosDefines.h>
#include <libcos/io/CosInputStream.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosTokenizer;

CosTokenizer * COS_Nullable
cos_tokenizer_alloc(CosInputStream *input_stream);

void
cos_tokenizer_free(CosTokenizer *tokenizer);

/**
 * Peek the next token without consuming it.
 *
 * @param tokenizer The tokenizer.
 *
 * @return The next token.
 */
CosToken *
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

/**
 * Consume the next token if it matches the given token type.
 *
 * @param tokenizer The tokenizer.
 * @param token_type The token type to match.
 *
 * @return The next token if it matches the given token type, otherwise @c NULL.
 */
CosToken * COS_Nullable
cos_tokenizer_get_next_token_if(CosTokenizer *tokenizer,
                                CosToken_Type token_type);

/**
 * Consume the next token if it matches the given keyword type.
 *
 * @param tokenizer The tokenizer.
 * @param keyword_type The keyword type to match.
 *
 * @return @c true if the next token matches the given keyword type, otherwise @c false.
 */
bool
cos_tokenizer_get_next_keyword_if(CosTokenizer *tokenizer,
                                  CosKeywordType keyword_type);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_TOKENIZER_H */
