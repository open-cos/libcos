//
// Created by david on 03/06/23.
//

#ifndef LIBCOS_COS_TOKENIZER_H
#define LIBCOS_COS_TOKENIZER_H

#include "parse/tokenizer/CosToken.h"

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

CosTokenizer * COS_Nullable
cos_tokenizer_alloc(CosInputStream *input_stream)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_tokenizer_free(CosTokenizer *tokenizer);

/**
 * Checks if there is a next token or if the end of the input stream has been reached.
 *
 * @param tokenizer The tokenizer.
 *
 * @return @c true if there is a next token, otherwise @c false.
 */
bool
cos_tokenizer_has_next_token(CosTokenizer *tokenizer);

/**
 * Peek the second next token without consuming it.
 *
 * @param tokenizer The tokenizer.
 *
 * @return The second next token.
 */
CosToken
cos_tokenizer_peek_next_token_(CosTokenizer *tokenizer);

bool
cos_tokenizer_get_next_token(CosTokenizer *tokenizer,
                             CosToken *out_token,
                             CosTokenValue * COS_Nullable *out_token_value,
                             CosError * COS_Nullable out_error);

bool
cos_tokenizer_peek_next_token(CosTokenizer *tokenizer,
                              CosToken *out_token,
                              const CosTokenValue * COS_Nullable * COS_Nullable out_token_value,
                              CosError * COS_Nullable out_error);

/**
 * @brief Peek the second next token without consuming it.
 *
 * @param tokenizer The tokenizer.
 * @param out_token The output parameter for the peeked token.
 * @param out_token_value The output parameter for the peeked token value, or @c NULL.
 * @param out_error The output parameter for the error, or @c NULL.
 *
 * @return @c true if the second next token was successfully peeked, otherwise @c false.
 */
bool
cos_tokenizer_peek_next_next_token(CosTokenizer *tokenizer,
                                   CosToken *out_token,
                                   const CosTokenValue * COS_Nullable * COS_Nullable out_token_value,
                                   CosError * COS_Nullable out_error);

/**
 * Consume the next token.
 *
 * @param tokenizer The tokenizer.
 *
 * @return The next token.
 */
CosToken
cos_tokenizer_next_token(CosTokenizer *tokenizer);

/**
 * Consume the next token if it matches the given token type.
 *
 * @param tokenizer The tokenizer.
 * @param type The token type to match.
 *
 * @return @c true if the next token matches the given token type, otherwise @c false.
 */
bool
cos_tokenizer_match_token(CosTokenizer *tokenizer,
                          CosToken_Type type)
    COS_PRECONDITION(type != CosToken_Type_Unknown);

/**
 * Consume the next token if it matches the given keyword type.
 *
 * @param tokenizer The tokenizer.
 * @param keyword_type The keyword type to match.
 *
 * @pre @p keyword_type is not @c CosKeywordType_Unknown.
 *
 * @return @c true if the next token matches the given keyword type, otherwise @c false.
 */
bool
cos_tokenizer_match_keyword(CosTokenizer *tokenizer,
                            CosKeywordType keyword_type)
    COS_PRECONDITION(keyword_type != CosKeywordType_Unknown);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_TOKENIZER_H */
