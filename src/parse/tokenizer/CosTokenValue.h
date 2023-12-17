//
// Created by david on 30/07/23.
//

#ifndef LIBCOS_COS_TOKEN_VALUE_H
#define LIBCOS_COS_TOKEN_VALUE_H

#include "common/CosData.h"
#include "common/CosString.h"

#include <stdbool.h>

typedef struct CosTokenValue CosTokenValue;

struct CosTokenValue {
    enum {
        CosTokenValue_Type_None,
        CosTokenValue_Type_Boolean,
        CosTokenValue_Type_String,
        CosTokenValue_Type_Data,
        CosTokenValue_Type_IntegerNumber,
        CosTokenValue_Type_RealNumber,
        CosTokenValue_Type_Keyword,
    } type;

    union {
        bool boolean;
        CosString *string;
        CosData *data;
        int integer_number;
        double real_number;
        CosKeywordType keyword;
    } value;
};

static inline CosTokenValue
cos_token_value_none(void)
{
    return (CosTokenValue){0};
}

static inline CosTokenValue
cos_token_value_boolean(bool value)
{
    return (CosTokenValue){
        .type = CosTokenValue_Type_Boolean,
        .value = {
            .boolean = value,
        },
    };
}

static inline CosTokenValue
cos_token_value_string(CosString *value)
{
    return (CosTokenValue){
        .type = CosTokenValue_Type_String,
        .value = {
            .string = value,
        },
    };
}

static inline CosTokenValue
cos_token_value_data(CosData *value)
{
    return (CosTokenValue){
        .type = CosTokenValue_Type_Data,
        .value = {
            .data = value,
        },
    };
}

static inline CosTokenValue
cos_token_value_integer_number(int value)
{
    return (CosTokenValue){
        .type = CosTokenValue_Type_IntegerNumber,
        .value = {
            .integer_number = value,
        },
    };
}

static inline CosTokenValue
cos_token_value_real_number(double value)
{
    return (CosTokenValue){
        .type = CosTokenValue_Type_RealNumber,
        .value = {
            .real_number = value,
        },
    };
}

static inline CosTokenValue
cos_token_value_keyword(CosKeywordType value)
{
    return (CosTokenValue){
        .type = CosTokenValue_Type_Keyword,
        .value = {
            .keyword = value,
        },
    };
}

static inline void
cos_token_value_reset(CosTokenValue *token_value)
{
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

static inline bool
cos_token_value_get_boolean(const CosTokenValue *token_value,
                            bool *result)
{
    if (!token_value || token_value->type != CosTokenValue_Type_Boolean) {
        return false;
    }

    if (result) {
        *result = token_value->value.boolean;
    }
    return true;
}

static inline bool
cos_token_value_get_string(const CosTokenValue *token_value, CosString **result)
{
    if (!token_value || token_value->type != CosTokenValue_Type_String) {
        return false;
    }

    if (result) {
        *result = token_value->value.string;
    }
    return true;
}

static inline bool
cos_token_value_get_data(const CosTokenValue *token_value,
                         CosData **result)
{
    if (!token_value || token_value->type != CosTokenValue_Type_Data) {
        return false;
    }

    if (result) {
        *result = token_value->value.data;
    }
    return true;
}

static inline bool
cos_token_value_get_integer_number(const CosTokenValue *token_value,
                                   int *result)
{
    if (!token_value || token_value->type != CosTokenValue_Type_IntegerNumber) {
        return false;
    }

    if (result) {
        *result = token_value->value.integer_number;
    }
    return true;
}

static inline bool
cos_token_value_get_real_number(const CosTokenValue *token_value,
                                double *result)
{
    if (!token_value || token_value->type != CosTokenValue_Type_RealNumber) {
        return false;
    }

    if (result) {
        *result = token_value->value.real_number;
    }
    return true;
}

#endif /* LIBCOS_COS_TOKEN_VALUE_H */
