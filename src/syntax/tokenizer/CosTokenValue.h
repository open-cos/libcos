/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COS_TOKEN_VALUE_H
#define LIBCOS_COS_TOKEN_VALUE_H

#include "libcos/common/CosData.h"
#include "libcos/common/CosDefines.h"
#include "libcos/common/CosString.h"
#include "libcos/common/CosTypes.h"
#include "libcos/syntax/CosKeywords.h"

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief Frees a token value.
 *
 * @param token_value The token value.
 */
void
cos_token_value_free(CosTokenValue *token_value)
    COS_DEALLOCATOR_FUNC;

/**
 * @brief Allocates a new token value.
 *
 * @return The new token value, or @c NULL if allocation failed.
 */
CosTokenValue * COS_Nullable
cos_token_value_alloc(void)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_token_value_free);

/**
 * @brief Frees the resources used by a token value and resets its fields.
 *
 * @param token_value The token value.
 */
void
cos_token_value_reset(CosTokenValue *token_value);

#pragma mark - Getters

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
                           const CosString * COS_Nullable * COS_Nonnull result);

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
                         const CosData * COS_Nullable * COS_Nonnull result);

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
 * @brief Gets the long integer number value of a token value.
 *
 * @param token_value The token value.
 * @param result A pointer to the variable in which to store the long integer number.
 *
 * @return @c true if the token value is a long integer number, @c false otherwise.
 */
bool
cos_token_value_get_long_integer_number(const CosTokenValue *token_value,
                                        long long *result);

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
 * @brief Sets the long integer number value of a token value.
 *
 * @param token_value The token value.
 * @param value The long integer number value.
 */
void
cos_token_value_set_long_integer_number(CosTokenValue *token_value,
                                        long long value);

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
                            CosString * COS_Nullable * COS_Nonnull result);

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
                          CosData * COS_Nullable * COS_Nonnull result);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_TOKEN_VALUE_H */
