//
// Created by david on 22/07/23.
//

#include "CosToken.h"

#include "parse/tokenizer/CosTokenValue.h"

bool
cos_token_get_boolean_value(const CosToken *token, bool *value)
{
    if (!token) {
        return false;
    }

    return cos_token_value_get_boolean(token->value,
                                       value);
}

bool
cos_token_get_integer_value(const CosToken *token,
                            int *value)
{
    if (!token) {
        return false;
    }

    return cos_token_value_get_integer_number(token->value,
                                              value);
}

bool
cos_token_get_real_value(const CosToken *token,
                         double *value)
{
    if (!token) {
        return false;
    }

    return cos_token_value_get_real_number(token->value,
                                           value);
}
