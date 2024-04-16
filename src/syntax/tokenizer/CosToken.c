/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "syntax/tokenizer/CosToken.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

CosToken *
cos_token_create(void)
{
    CosToken *token = NULL;
    CosTokenValue *value = NULL;

    token = calloc(1, sizeof(CosToken));
    if (!token) {
        goto failure;
    }

    value = cos_token_value_alloc();
    if (!value) {
        goto failure;
    }

    token->type = CosToken_Type_Unknown;
    token->value = value;

    return token;

failure:
    if (token) {
        free(token);
    }
    if (value) {
        cos_token_value_free(value);
    }
    return NULL;
}

void
cos_token_destroy(CosToken *token)
{
    if (!token) {
        return;
    }

    cos_token_value_free(token->value);
    free(token);
}

void
cos_token_reset(CosToken *token)
{
    if (!token) {
        return;
    }

    token->type = CosToken_Type_Unknown;
    cos_token_value_reset(token->value);
}

COS_ASSUME_NONNULL_END
