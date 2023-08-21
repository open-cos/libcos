//
// Created by david on 22/07/23.
//

#include "CosToken.h"

#include "parse/tokenizer/CosTokenValue.h"

#include <string.h>

char *
cos_token_copy_string_value(const CosToken *token,
                            size_t *out_size)
{
    if (!token) {
        return NULL;
    }

    const char *string_value;
    if (cos_token_value_get_string(token->value,
                               &string_value)) {

    }

    return NULL;
}

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
