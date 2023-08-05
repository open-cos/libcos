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

static inline bool
cos_token_value_get_boolean(const CosTokenValue *value,
                            bool *result)
{
    if (!value || value->type != CosTokenValue_Type_Boolean) {
        return false;
    }

    if (result) {
        *result = value->value.boolean;
    }
    return true;
}

static inline bool
cos_token_value_get_string(const CosTokenValue *value,
                           const char **result)
{
    if (!value || value->type != CosTokenValue_Type_String) {
        return false;
    }

    if (result) {
        *result = value->value.string;
    }
    return true;
}

static inline bool
cos_token_value_get_integer_number(const CosTokenValue *value,
                                   int *result)
{
    if (!value || value->type != CosTokenValue_Type_IntegerNumber) {
        return false;
    }

    if (result) {
        *result = value->value.integer_number;
    }
    return true;
}

static inline bool
cos_token_value_get_real_number(const CosTokenValue *value,
                                double *result)
{
    if (!value || value->type != CosTokenValue_Type_RealNumber) {
        return false;
    }

    if (result) {
        *result = value->value.real_number;
    }
    return true;
}

#endif /* LIBCOS_COS_TOKEN_VALUE_H */
