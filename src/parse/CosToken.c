/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "CosToken.h"

#include "common/Assert.h"

COS_ASSUME_NONNULL_BEGIN

void
cos_token_reset(CosToken *token)
{
    COS_PARAMETER_ASSERT(token != NULL);
    if (!token) {
        return;
    }

    token->type = CosToken_Type_Unknown;
    cos_token_value_reset(token->value);
}

COS_ASSUME_NONNULL_END
