/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "syntax/tokenizer/CosToken.h"

#include "CosTokenValue.h"
#include "common/Assert.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

CosToken *
cos_token_create(void)
{
    CosToken *token = NULL;

    token = calloc(1, sizeof(CosToken));
    if (!token) {
        goto failure;
    }

    token->type = CosToken_Type_Unknown;
    token->offset = 0;
    token->length = 0;
    token->value = (CosTokenValue){0};

    return token;

failure:
    if (token) {
        free(token);
    }
    return NULL;
}

void
cos_token_destroy(CosToken *token)
{
    if (!token) {
        return;
    }

    free(token);
}

void
cos_token_reset(CosToken *token)
{
    COS_PARAMETER_ASSERT(token != NULL);
    if (!token) {
        return;
    }

    token->type = CosToken_Type_Unknown;
    token->offset = 0;
    token->length = 0;
    cos_token_value_reset(&token->value);
}

bool
cos_token_get_integer_value(const CosToken *token,
                            int *out_value)
{
    COS_PARAMETER_ASSERT(token != NULL);
    COS_PARAMETER_ASSERT(out_value != NULL);
    if (!token || !out_value) {
        return false;
    }

    return cos_token_value_get_integer_number(&token->value,
                                              out_value);
}

CosData *
cos_token_move_data_value(CosToken *token)
{
    COS_PARAMETER_ASSERT(token != NULL);
    if (COS_UNLIKELY(!token)) {
        return NULL;
    }

    CosData *data = NULL;
    cos_token_value_take_data(&token->value,
                              &data);
    return data;
}

CosString *
cos_token_move_string_value(CosToken *token)
{
    COS_PARAMETER_ASSERT(token != NULL);
    if (COS_UNLIKELY(!token)) {
        return NULL;
    }

    CosString *string = NULL;
    cos_token_value_take_string(&token->value,
                                &string);
    return string;
}

COS_ASSUME_NONNULL_END
