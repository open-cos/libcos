/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_SYNTAX_TOKENIZER_COS_TOKENIZER_H
#define LIBCOS_SYNTAX_TOKENIZER_COS_TOKENIZER_H

#include "syntax/tokenizer/CosToken.h"

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief Deallocates a tokenizer.
 *
 * @param tokenizer The tokenizer.
 */
void
cos_tokenizer_destroy(CosTokenizer *tokenizer)
    COS_DEALLOCATOR_FUNC;

CosTokenizer * COS_Nullable
cos_tokenizer_create(CosStream *input_stream)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_tokenizer_destroy);

void
cos_tokenizer_reset(CosTokenizer *tokenizer);

/**
 * Checks if there is a next token or if the end of the input stream has been reached.
 *
 * @param tokenizer The tokenizer.
 *
 * @return @c true if there is a next token, otherwise @c false.
 */
bool
cos_tokenizer_has_next_token(CosTokenizer *tokenizer);

const CosToken * COS_Nullable
cos_tokenizer_peek_next_token(CosTokenizer *tokenizer,
                              CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(2);

CosToken * COS_Nullable
cos_tokenizer_get_next_token(CosTokenizer *tokenizer,
                             CosError * COS_Nullable out_error)
    //    COS_WARN_UNUSED_RESULT
    COS_OWNERSHIP_RETURNS
    COS_ATTR_ACCESS_WRITE_ONLY(2);

void
cos_tokenizer_release_token(CosTokenizer *tokenizer,
                            CosToken *token);

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

/**
 * Returns the next token if it matches the given token type.
 *
 * @param tokenizer The tokenizer.
 * @param type The token type to match.
 *
 * @return The next token if it matches the given token type, otherwise @c NULL.
 */
CosToken * COS_Nullable
cos_tokenizer_match_next_token(CosTokenizer *tokenizer,
                               CosToken_Type type,
                               CosError * COS_Nullable out_error)
    COS_OWNERSHIP_RETURNS
    COS_ATTR_ACCESS_WRITE_ONLY(3);

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

size_t
cos_tokenizer_skip_characters(CosTokenizer *tokenizer,
                              size_t count);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_SYNTAX_TOKENIZER_COS_TOKENIZER_H */
