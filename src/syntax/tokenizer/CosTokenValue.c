/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "CosTokenValue.h"

#include "CosToken.h"
#include "common/Assert.h"

#include <stdlib.h>

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
        case CosTokenValue_Type_IntegerNumber:
        case CosTokenValue_Type_LongIntegerNumber:
        case CosTokenValue_Type_RealNumber:
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

// MARK: - Getters

bool
cos_token_value_get_string(const CosTokenValue *token_value,
                           const CosString * COS_Nullable *result)
{
    COS_PARAMETER_ASSERT(token_value != NULL);
    COS_PARAMETER_ASSERT(result != NULL);
    if (!token_value || token_value->type != CosTokenValue_Type_String) {
        return false;
    }

    if (result) {
        *result = token_value->value.string;
    }
    return true;
}

bool
cos_token_value_get_data(const CosTokenValue *token_value,
                         const CosData * COS_Nullable *result)
{
    COS_PARAMETER_ASSERT(token_value != NULL);
    COS_PARAMETER_ASSERT(result != NULL);
    if (!token_value || token_value->type != CosTokenValue_Type_Data) {
        return false;
    }

    if (result) {
        *result = token_value->value.data;
    }
    return true;
}

bool
cos_token_value_get_integer_number(const CosTokenValue *token_value,
                                   int *result)
{
    COS_PARAMETER_ASSERT(token_value != NULL);
    COS_PARAMETER_ASSERT(result != NULL);
    if (!token_value || token_value->type != CosTokenValue_Type_IntegerNumber) {
        return false;
    }

    if (result) {
        *result = token_value->value.integer_number;
    }
    return true;
}

bool
cos_token_value_get_long_integer_number(const CosTokenValue *token_value,
                                        long long *result)
{
    COS_PARAMETER_ASSERT(token_value != NULL);
    COS_PARAMETER_ASSERT(result != NULL);
    if (!token_value || token_value->type != CosTokenValue_Type_LongIntegerNumber) {
        return false;
    }

    if (result) {
        *result = token_value->value.long_integer_number;
    }
    return true;
}

bool
cos_token_value_get_real_number(const CosTokenValue *token_value,
                                double *result)
{
    COS_PARAMETER_ASSERT(token_value != NULL);
    COS_PARAMETER_ASSERT(result != NULL);
    if (!token_value || token_value->type != CosTokenValue_Type_RealNumber) {
        return false;
    }

    if (result) {
        *result = token_value->value.real_number;
    }
    return true;
}

// MARK: - Setters

void
cos_token_value_set_string(CosTokenValue *token_value,
                           CosString *value)
{
    COS_PARAMETER_ASSERT(token_value != NULL);
    COS_PARAMETER_ASSERT(value != NULL);
    if (!token_value) {
        return;
    }

    cos_token_value_reset(token_value);

    token_value->type = CosTokenValue_Type_String;
    token_value->value.string = value;
}

void
cos_token_value_set_data(CosTokenValue *token_value,
                         CosData *value)
{
    COS_PARAMETER_ASSERT(token_value != NULL);
    COS_PARAMETER_ASSERT(value != NULL);
    if (!token_value) {
        return;
    }

    cos_token_value_reset(token_value);

    token_value->type = CosTokenValue_Type_Data;
    token_value->value.data = value;
}

void
cos_token_value_set_integer_number(CosTokenValue *token_value,
                                   int value)
{
    COS_PARAMETER_ASSERT(token_value != NULL);
    if (!token_value) {
        return;
    }

    cos_token_value_reset(token_value);

    token_value->type = CosTokenValue_Type_IntegerNumber;
    token_value->value.integer_number = value;
}

void
cos_token_value_set_long_integer_number(CosTokenValue *token_value,
                                        long long value)
{
    COS_PARAMETER_ASSERT(token_value != NULL);
    if (!token_value) {
        return;
    }

    cos_token_value_reset(token_value);

    token_value->type = CosTokenValue_Type_LongIntegerNumber;
    token_value->value.long_integer_number = value;
}

void
cos_token_value_set_real_number(CosTokenValue *token_value,
                                double value)
{
    COS_PARAMETER_ASSERT(token_value != NULL);
    if (!token_value) {
        return;
    }

    cos_token_value_reset(token_value);

    token_value->type = CosTokenValue_Type_RealNumber;
    token_value->value.real_number = value;
}

bool
cos_token_value_take_string(CosTokenValue *token_value,
                            CosString * COS_Nullable *result)
{
    COS_PARAMETER_ASSERT(token_value != NULL);
    COS_PARAMETER_ASSERT(result != NULL);
    if (!token_value || token_value->type != CosTokenValue_Type_String) {
        return false;
    }

    if (result) {
        *result = token_value->value.string;

        // Reset the token value type, but don't free the string.
        token_value->type = CosTokenValue_Type_None;

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
    COS_PARAMETER_ASSERT(token_value != NULL);
    COS_PARAMETER_ASSERT(result != NULL);
    if (!token_value || token_value->type != CosTokenValue_Type_Data) {
        return false;
    }

    if (result) {
        *result = token_value->value.data;

        // Reset the token value type, but don't free the data.
        token_value->type = CosTokenValue_Type_None;

        return true;
    }
    else {
        return false;
    }
}

COS_ASSUME_NONNULL_END
