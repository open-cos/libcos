//
// Created by david on 30/07/23.
//

#ifndef LIBCOS_COS_TOKEN_VALUE_H
#define LIBCOS_COS_TOKEN_VALUE_H

#include "common/CosString.h"
#include "libcos/common/CosData.h"
#include "syntax/CosSyntax.h"

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

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

/**
 * @brief Frees the resources used by a token value and resets its fields.
 *
 * @param token_value The token value.
 */
void
cos_token_value_reset(CosTokenValue *token_value);

#pragma mark - Getters

/**
 * @brief Gets the boolean value of a token value.
 *
 * @param token_value The token value.
 * @param result A pointer to the variable to store the boolean value in.
 *
 * @return @c true if the token value is a boolean, @c false otherwise.
 */
bool
cos_token_value_get_boolean(const CosTokenValue *token_value,
                            bool *result);

/**
 * @brief Gets the string value of a token value.
 *
 * @param token_value The token value.
 * @param result A pointer to the variable in which to store the string pointer.
 *
 * @return @c true if the token value is a string, @c false otherwise.
 */
bool
cos_token_value_get_string(const CosTokenValue *token_value,
                           const CosString * COS_Nullable *result);

/**
 * @brief Gets the data value of a token value.
 *
 * @param token_value The token value.
 * @param result A pointer to the variable in which to store the data pointer.
 *
 * @return @c true if the token value is a data, @c false otherwise.
 */
bool
cos_token_value_get_data(const CosTokenValue *token_value,
                         const CosData * COS_Nullable *result);

/**
 * @brief Gets the integer number value of a token value.
 *
 * @param token_value The token value.
 * @param result A pointer to the variable in which to store the integer number.
 *
 * @return @c true if the token value is an integer number, @c false otherwise.
 */
bool
cos_token_value_get_integer_number(const CosTokenValue *token_value,
                                   int *result);

/**
 * @brief Gets the real number value of a token value.
 *
 * @param token_value The token value.
 * @param result A pointer to the variable in which to store the real number.
 *
 * @return @c true if the token value is a real number, @c false otherwise.
 */
bool
cos_token_value_get_real_number(const CosTokenValue *token_value,
                                double *result);

/**
 * @brief Gets the keyword value of a token value.
 *
 * @param token_value The token value.
 * @param result A pointer to the variable in which to store the keyword.
 *
 * @return @c true if the token value is a keyword, @c false otherwise.
 */
bool
cos_token_value_get_keyword(const CosTokenValue *token_value,
                            CosKeywordType *result);

#pragma mark - Setters

/**
 * @brief Sets the boolean value of a token value.
 *
 * @param token_value The token value.
 * @param value The boolean value.
 */
void
cos_token_value_set_boolean(CosTokenValue *token_value,
                            bool value);

/**
 * @brief Sets the string value of a token value.
 *
 * @param token_value The token value.
 * @param value The string value.
 */
void
cos_token_value_set_string(CosTokenValue *token_value,
                           CosString *value);

/**
 * @brief Sets the data value of a token value.
 *
 * @param token_value The token value.
 * @param value The data value.
 */
void
cos_token_value_set_data(CosTokenValue *token_value,
                         CosData *value);

/**
 * @brief Sets the integer number value of a token value.
 *
 * @param token_value The token value.
 * @param value The integer number value.
 */
void
cos_token_value_set_integer_number(CosTokenValue *token_value,
                                   int value);

/**
 * @brief Sets the real number value of a token value.
 *
 * @param token_value The token value.
 * @param value The real number value.
 */
void
cos_token_value_set_real_number(CosTokenValue *token_value,
                                double value);

/**
 * @brief Sets the keyword value of a token value.
 *
 * @param token_value The token value.
 * @param value The keyword value.
 */
void
cos_token_value_set_keyword(CosTokenValue *token_value,
                            CosKeywordType value);

#pragma mark - Ownership transfer

/**
 * @brief Transfers ownership of the value's string to the caller.
 *
 * @param token_value The token value.
 * @param result A pointer to the variable to store the string pointer in.
 *
 * @return @c true if ownership of the string was transferred, @c false otherwise.
 */
bool
cos_token_value_take_string(CosTokenValue *token_value,
                            CosString * COS_Nullable *result);

/**
 * @brief Transfers ownership of the value's data to the caller.
 *
 * @param token_value The token value.
 * @param result A pointer to the variable to store the data pointer in.
 *
 * @return @c true if ownership of the data was transferred, @c false otherwise.
 */
bool
cos_token_value_take_data(CosTokenValue *token_value,
                          CosData * COS_Nullable *result);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_TOKEN_VALUE_H */
