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
typedef struct CosTokenizer CosTokenizer;

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

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_TOKENIZER_H */
