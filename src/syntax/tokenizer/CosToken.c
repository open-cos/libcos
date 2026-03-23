/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "common/Assert.h"
#include "common/CharacterSet.h"

#include <libcos/syntax/tokenizer/CosToken.h>
#include <libcos/syntax/tokenizer/CosTokenValue.h>

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

bool
cos_token_whitespace_is_single_space(const CosTokenWhitespace *ws)
{
    COS_API_PARAM_CHECK(ws != NULL);
    if (COS_UNLIKELY(!ws)) {
        return false;
    }

    return (ws->char_count == 1 &&
            !ws->has_comment &&
            ws->bytes_count == 1 &&
            ws->bytes[0] == CosCharacterSet_Space);
}

bool
cos_token_whitespace_is_eol(const CosTokenWhitespace *ws)
{
    COS_API_PARAM_CHECK(ws != NULL);
    if (COS_UNLIKELY(!ws)) {
        return false;
    }

    if (ws->has_comment || ws->bytes_count == 0) {
        return false;
    }

    /* LF only */
    if (ws->char_count == 1 && ws->bytes[0] == CosCharacterSet_LineFeed) {
        return true;
    }

    /* CR+LF */
    if (ws->char_count == 2 &&
        ws->bytes_count >= 2 &&
        ws->bytes[0] == CosCharacterSet_CarriageReturn &&
        ws->bytes[1] == CosCharacterSet_LineFeed) {
        return true;
    }

    return false;
}

bool
cos_token_whitespace_is_bare_cr(const CosTokenWhitespace *ws)
{
    COS_API_PARAM_CHECK(ws != NULL);
    if (COS_UNLIKELY(!ws)) {
        return false;
    }

    return (ws->char_count == 1 &&
            !ws->has_comment &&
            ws->bytes_count == 1 &&
            ws->bytes[0] == CosCharacterSet_CarriageReturn);
}

void
cos_token_reset(CosToken *token)
{
    COS_API_PARAM_CHECK(token != NULL);
    if (!token) {
        return;
    }

    token->type = CosToken_Type_Unknown;
    token->offset = 0;
    token->length = 0;
    cos_token_value_reset(&token->value);
    memset(&token->leading_whitespace, 0, sizeof(token->leading_whitespace));
}

bool
cos_token_get_integer_value(const CosToken *token,
                            int *out_value)
{
    COS_API_PARAM_CHECK(token != NULL);
    COS_API_PARAM_CHECK(out_value != NULL);
    if (!token || !out_value) {
        return false;
    }

    return cos_token_value_get_integer_number(&token->value,
                                              out_value);
}

CosData *
cos_token_move_data_value(CosToken *token)
{
    COS_API_PARAM_CHECK(token != NULL);
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
    COS_API_PARAM_CHECK(token != NULL);
    if (COS_UNLIKELY(!token)) {
        return NULL;
    }

    CosString *string = NULL;
    cos_token_value_take_string(&token->value,
                                &string);
    return string;
}

COS_ASSUME_NONNULL_END
