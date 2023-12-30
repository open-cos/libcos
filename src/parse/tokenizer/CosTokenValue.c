/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "CosTokenValue.h"

#include "common/Assert.h"

COS_ASSUME_NONNULL_BEGIN

void
cos_token_value_reset(CosTokenValue *token_value)
{
    COS_PARAMETER_ASSERT(token_value != NULL);
    if (!token_value) {
        return;
    }

    switch (token_value->type) {
        case CosTokenValue_Type_None:
        case CosTokenValue_Type_Boolean:
        case CosTokenValue_Type_IntegerNumber:
        case CosTokenValue_Type_RealNumber:
        case CosTokenValue_Type_Keyword:
            break;

        case CosTokenValue_Type_String: {
            cos_string_free(token_value->value.string);
        } break;

        case CosTokenValue_Type_Data: {
            cos_data_free(token_value->value.data);
        } break;
    }

    token_value->type = CosTokenValue_Type_None;
}

bool
cos_token_value_take_string(CosTokenValue *token_value,
                            CosString * COS_Nullable *result)
{
    if (!token_value || token_value->type != CosTokenValue_Type_String) {
        return false;
    }

    if (result) {
        *result = token_value->value.string;

        // Clear (but not free) the token value.
        *token_value = cos_token_value_none();

        return true;
    }
    else {
        return false;
    }
}

bool
cos_token_value_take_data(CosTokenValue *token_value,
                          CosData * COS_Nullable *result)
{
    if (!token_value || token_value->type != CosTokenValue_Type_Data) {
        return false;
    }

    if (result) {
        *result = token_value->value.data;

        // Clear (but not free) the token value.
        *token_value = cos_token_value_none();

        return true;
    }
    else {
        return false;
    }
}

COS_ASSUME_NONNULL_END
