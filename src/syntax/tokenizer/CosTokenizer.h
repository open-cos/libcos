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

bool
cos_tokenizer_get_next_token(CosTokenizer *tokenizer,
                             CosToken *out_token,
                             CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(2)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_SYNTAX_TOKENIZER_COS_TOKENIZER_H */
