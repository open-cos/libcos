//
// Created by david on 30/07/23.
//

#ifndef LIBCOS_COS_TOKEN_VALUE_H
#define LIBCOS_COS_TOKEN_VALUE_H

#include <stdbool.h>

typedef struct CosTokenValue CosTokenValue;

struct CosTokenValue {
    enum {
        CosTokenValue_Type_Boolean,
        CosTokenValue_Type_String,
        CosTokenValue_Type_IntegerNumber,
        CosTokenValue_Type_RealNumber,
    } type;

    union {
        bool boolean;
        const char *string;
        int integer_number;
        double real_number;
    } value;
};

static inline CosTokenValue
cos_token_value_make_boolean(bool value)
{
    return (CosTokenValue){
        .type = CosTokenValue_Type_Boolean,
        .value = {
            .boolean = value,
        },
    };
}

static inline CosTokenValue
cos_token_value_make_string(const char *value)
{
    return (CosTokenValue){
        .type = CosTokenValue_Type_String,
        .value = {
            .string = value,
        },
    };
}

static inline CosTokenValue
cos_token_value_make_integer_number(int value)
{
    return (CosTokenValue){
        .type = CosTokenValue_Type_IntegerNumber,
        .value = {
            .integer_number = value,
        },
    };
}

static inline CosTokenValue
cos_token_value_make_real_number(double value)
{
    return (CosTokenValue){
        .type = CosTokenValue_Type_RealNumber,
        .value = {
            .real_number = value,
        },
    };
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
cos_token_value_get_string(const CosTokenValue *token_value,
                           const char **result)
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
