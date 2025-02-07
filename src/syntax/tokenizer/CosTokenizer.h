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

/**
 * @brief Creates a new tokenizer.
 *
 * @param input_stream The input stream to tokenize.
 *
 * @return The new tokenizer, or @c NULL if an error occurred.
 */
CosTokenizer * COS_Nullable
cos_tokenizer_create(CosStream *input_stream)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_tokenizer_destroy)
    COS_OWNERSHIP_HOLDS(1);

/**
 * @brief Resets the tokenizer to an initial state.
 *
 * @param tokenizer The tokenizer to reset.
 */
void
cos_tokenizer_reset(CosTokenizer *tokenizer);

/**
 * @brief Gets the next token from the tokenizer.
 *
 * @param tokenizer The tokenizer.
 * @param out_token A pointer to the variable in which to store the token.
 * @param out_error The error object to set if an error occurs.
 *
 * @return @c true if a token was successfully read, @c false otherwise.
 */
bool
cos_tokenizer_get_next_token(CosTokenizer *tokenizer,
                             CosToken *out_token,
                             CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(2)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_SYNTAX_TOKENIZER_COS_TOKENIZER_H */
