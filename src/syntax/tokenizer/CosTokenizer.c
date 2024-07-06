/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "CosTokenizer.h"

#include "common/Assert.h"
#include "common/CharacterSet.h"
#include "io/CosInputStreamReader.h"

#include "libcos/common/CosError.h"
#include "libcos/common/CosNumber.h"
#include "libcos/common/CosRingBuffer.h"
#include "libcos/common/CosString.h"
#include "libcos/syntax/CosLimits.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

#define COS_TOKENIZER_MAX_PEEKED_TOKENS 2

struct CosTokenizer {
    CosInputStreamReader *input_stream_reader;

    CosRingBuffer *peeked_tokens;
    CosRingBuffer *free_tokens;

    bool strict;
};

static CosToken * COS_Nullable
cos_acquire_token_(CosTokenizer *tokenizer)
    COS_OWNERSHIP_RETURNS;

static void
cos_release_token_(CosTokenizer *tokenizer,
                   CosToken *token)
    COS_OWNERSHIP_TAKES(2);

static CosToken * COS_Nullable
cos_get_next_token_(CosTokenizer *tokenizer,
                    CosError * COS_Nullable out_error)
    COS_OWNERSHIP_RETURNS
    COS_ATTR_ACCESS_WRITE_ONLY(2);

static void
cos_consume_next_token_(CosTokenizer *tokenizer);

static CosToken * COS_Nullable
cos_tokenizer_read_next_token_(CosTokenizer *tokenizer,
                               CosError * COS_Nullable out_error)
    COS_OWNERSHIP_RETURNS
    COS_ATTR_ACCESS_WRITE_ONLY(2);

static inline int
cos_tokenizer_get_next_char_(CosTokenizer *tokenizer);

static inline int
cos_tokenizer_peek_next_char_(CosTokenizer *tokenizer);

/**
 * Advances the tokenizer if the next character matches the given character.
 *
 * @param tokenizer The tokenizer.
 * @param character The character to match.
 *
 * @return true if the next character matches the given character, false otherwise.
 */
static bool
cos_tokenizer_match_(CosTokenizer *tokenizer, char character);

static void
cos_tokenizer_skip_whitespace_and_comments_(CosTokenizer *tokenizer);

static void
cos_tokenizer_skip_comment_(CosTokenizer *tokenizer);

static bool
cos_tokenizer_read_name_(CosTokenizer *tokenizer,
                         CosString *string,
                         CosError *error)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

static int
cos_tokenizer_handle_name_hex_escape_sequence_(CosTokenizer *tokenizer);

static bool
cos_tokenizer_read_literal_string_(CosTokenizer *tokenizer,
                                   CosData *data);

/**
 * Handles an escape sequence in a literal string.
 *
 * @param tokenizer The tokenizer.
 * @param result The output parameter for the escaped character.
 *
 * @return @c true if a character was written to @p result, @c false otherwise.
 */
static bool
cos_tokenizer_handle_literal_string_escape_sequence_(CosTokenizer *tokenizer,
                                                     unsigned char *result)
    COS_ATTR_ACCESS_WRITE_ONLY(2);

/**
 * Reads an octal escape sequence.
 *
 * @note High-order overflow is ignored.
 *
 * @param tokenizer The tokenizer.
 *
 * @return The value of the octal escape sequence.
 */
static int
cos_tokenizer_read_octal_escape_sequence_(CosTokenizer *tokenizer);

static bool
cos_tokenizer_read_hex_string_(CosTokenizer *tokenizer,
                               CosData *data,
                               CosError *error);

static bool
cos_read_number_(CosTokenizer *tokenizer,
                 CosNumber *out_number,
                 CosError *out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(2)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

static bool
cos_tokenizer_read_token_(CosTokenizer *tokenizer,
                          CosString *string,
                          CosError *error);

static CosToken_Type
cos_keyword_token_type_from_string_(CosStringRef string);

#pragma mark - Public

CosTokenizer *
cos_tokenizer_alloc(CosStream *input_stream)
{
    COS_PARAMETER_ASSERT(input_stream != NULL);

    CosTokenizer *tokenizer = NULL;
    CosInputStreamReader *input_stream_reader = NULL;

    CosRingBuffer *peeked_tokens = NULL;
    CosRingBuffer *free_tokens = NULL;

    tokenizer = calloc(1, sizeof(CosTokenizer));
    if (!tokenizer) {
        goto failure;
    }

    input_stream_reader = cos_input_stream_reader_create(input_stream);
    if (!input_stream_reader) {
        goto failure;
    }

    peeked_tokens = cos_ring_buffer_create(sizeof(CosToken *),
                                           COS_TOKENIZER_MAX_PEEKED_TOKENS);
    if (!peeked_tokens) {
        goto failure;
    }
    free_tokens = cos_ring_buffer_create(sizeof(CosToken *),
                                         COS_TOKENIZER_MAX_PEEKED_TOKENS);
    if (!free_tokens) {
        goto failure;
    }

    tokenizer->input_stream_reader = input_stream_reader;
    tokenizer->peeked_tokens = peeked_tokens;
    tokenizer->free_tokens = free_tokens;

    return tokenizer;

failure:
    if (tokenizer) {
        free(tokenizer);
    }
    if (input_stream_reader) {
        cos_input_stream_reader_destroy(input_stream_reader);
    }
    if (peeked_tokens) {
        cos_ring_buffer_destroy(peeked_tokens);
    }
    if (free_tokens) {
        cos_ring_buffer_destroy(free_tokens);
    }

    return NULL;
}

void
cos_tokenizer_free(CosTokenizer *tokenizer)
{
    if (COS_UNLIKELY(!tokenizer)) {
        return;
    }

    while (cos_ring_buffer_get_count(tokenizer->peeked_tokens) > 0) {
        CosToken *token = NULL;
        if (cos_ring_buffer_pop_front(tokenizer->peeked_tokens,
                                      (void *)&token)) {
            cos_token_destroy(token);
        }
    }
    cos_ring_buffer_destroy(tokenizer->peeked_tokens);

    while (cos_ring_buffer_get_count(tokenizer->free_tokens) > 0) {
        CosToken *token = NULL;
        if (cos_ring_buffer_pop_front(tokenizer->free_tokens,
                                      (void *)&token)) {
            cos_token_destroy(token);
        }
    }
    cos_ring_buffer_destroy(tokenizer->free_tokens);

    cos_input_stream_reader_destroy(tokenizer->input_stream_reader);

    free(tokenizer);
}

void
cos_tokenizer_reset(CosTokenizer *tokenizer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    if (COS_UNLIKELY(!tokenizer)) {
        return;
    }

    cos_input_stream_reader_reset(tokenizer->input_stream_reader);

    // Clear all the peeked tokens.
    while (cos_ring_buffer_get_count(tokenizer->peeked_tokens) > 0) {
        CosToken *token = NULL;
        if (cos_ring_buffer_pop_front(tokenizer->peeked_tokens,
                                      (void *)&token)) {
            cos_release_token_(tokenizer, token);
        }
    }
}

bool
cos_tokenizer_has_next_token(CosTokenizer *tokenizer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    const CosToken *peeked_token = cos_tokenizer_peek_next_token(tokenizer,
                                                                 NULL);
    if (!peeked_token) {
        return false;
    }

    return (peeked_token->type != CosToken_Type_EOF);
}

const CosToken *
cos_tokenizer_peek_next_token(CosTokenizer *tokenizer,
                              CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    if (!tokenizer) {
        return NULL;
    }

    CosToken *token = NULL;

    // Check if we already have a token in the buffer.
    if (cos_ring_buffer_get_first_item(tokenizer->peeked_tokens,
                                       (void *)&token)) {
        COS_ASSERT(token != NULL, "Expected a token");
        if (!token) {
            goto failure;
        }
    }
    else {
        // We don't have a token in the buffer, so we need to read the next token.
        token = cos_get_next_token_(tokenizer,
                                    out_error);
        if (!token) {
            goto failure;
        }

        cos_ring_buffer_push_back(tokenizer->peeked_tokens,
                                  (void *)&token,
                                  out_error);
    }
    return token;

failure:

    return NULL;
}

CosToken *
cos_tokenizer_get_next_token(CosTokenizer *tokenizer,
                             CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    CosToken *token = NULL;

    // Check if we already have a token in the buffer.
    if (cos_ring_buffer_pop_front(tokenizer->peeked_tokens,
                                  (void *)&token)) {
        COS_ASSERT(token != NULL, "Expected a token");
        if (!token) {
            goto failure;
        }
    }
    else {
        // We don't have a token in the buffer, so we need to read the next token.
        token = cos_get_next_token_(tokenizer,
                                    out_error);
        if (!token) {
            goto failure;
        }
    }

    return token;

failure:
    return NULL;
}

void
cos_tokenizer_release_token(CosTokenizer *tokenizer,
                            CosToken *token)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    COS_PARAMETER_ASSERT(token != NULL);
    if (!tokenizer || !token) {
        return;
    }

    cos_release_token_(tokenizer, token);
}

static CosToken *
cos_acquire_token_(CosTokenizer *tokenizer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    CosToken *token = NULL;

    // Check if we have an available token.
    if (cos_ring_buffer_pop_front(tokenizer->free_tokens,
                                  (void *)&token)) {
        COS_ASSERT(token != NULL, "Expected a token");
        return token;
    }

    // Allocate a new token.
    return cos_token_create();
}

static void
cos_release_token_(CosTokenizer *tokenizer,
                   CosToken *token)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    COS_PARAMETER_ASSERT(token != NULL);
    if (!tokenizer || !token) {
        return;
    }

    cos_token_reset(token);

    if (!cos_ring_buffer_push_back(tokenizer->free_tokens,
                                   (void *)&token,
                                   NULL)) {
        // Error: out of memory.
        cos_token_destroy(token);
    }
}

static CosToken *
cos_get_next_token_(CosTokenizer *tokenizer,
                    CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    if (!tokenizer) {
        return NULL;
    }

    return cos_tokenizer_read_next_token_(tokenizer,
                                          out_error);
}

bool
cos_tokenizer_peek_next_next_token(CosTokenizer *tokenizer,
                                   CosToken *out_token,
                                   const CosTokenValue * COS_Nullable * COS_Nullable out_token_value,
                                   CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    COS_PARAMETER_ASSERT(out_token != NULL);
    if (!tokenizer || !out_token) {
        return false;
    }

    CosToken *token = NULL;

    // Check if we already have a second peeked token.
    if (cos_ring_buffer_get_count(tokenizer->peeked_tokens) > 1) {
        // We have at least 2 peeked tokens.
        if (!cos_ring_buffer_get_item(tokenizer->peeked_tokens,
                                      1,
                                      (void *)&token,
                                      NULL)) {
            goto failure;
        }
        COS_ASSERT(token != NULL, "Expected a token");
        if (!token) {
            goto failure;
        }
    }
    else {
        // We don't have enough peeked tokens, so we need to read the next token.

        token = cos_tokenizer_read_next_token_(tokenizer,
                                               out_error);
        if (!token) {
            goto failure;
        }

        cos_ring_buffer_push_back(tokenizer->peeked_tokens,
                                  (void *)&token,
                                  NULL);
    }

    if (out_token) {
        *out_token = *token;
    }
    if (out_token_value) {
        *out_token_value = token->value;
    }

    return true;

failure:

    return false;
}

static void
cos_consume_next_token_(CosTokenizer *tokenizer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    if (!tokenizer) {
        return;
    }

    CosToken *token = cos_get_next_token_(tokenizer, NULL);
    if (token) {
        cos_release_token_(tokenizer, token);
    }
}

CosToken *
cos_tokenizer_match_next_token(CosTokenizer *tokenizer,
                               CosToken_Type type,
                               CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    COS_PARAMETER_ASSERT(type != CosToken_Type_Unknown);

    const CosToken * const peeked_token = cos_tokenizer_peek_next_token(tokenizer,
                                                                        NULL);
    if (!peeked_token || cos_token_get_type(peeked_token) != type) {
        return NULL;
    }

    return cos_tokenizer_get_next_token(tokenizer,
                                        out_error);
}

bool
cos_tokenizer_match_token(CosTokenizer *tokenizer,
                          CosToken_Type type)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    COS_PARAMETER_ASSERT(type != CosToken_Type_Unknown);

    const CosToken *peeked_token = cos_tokenizer_peek_next_token(tokenizer,
                                                                 NULL);
    if (!peeked_token) {
        return false;
    }

    if (peeked_token->type == type) {
        // Consume the token.
        cos_consume_next_token_(tokenizer);
        return true;
    }

    return false;
}

bool
cos_tokenizer_match_keyword(CosTokenizer *tokenizer,
                            CosToken_Type keyword_type)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    COS_PARAMETER_ASSERT(keyword_type == CosToken_Type_True ||
                         keyword_type == CosToken_Type_False ||
                         keyword_type == CosToken_Type_Null ||
                         keyword_type == CosToken_Type_R ||
                         keyword_type == CosToken_Type_Obj ||
                         keyword_type == CosToken_Type_EndObj ||
                         keyword_type == CosToken_Type_Stream ||
                         keyword_type == CosToken_Type_EndStream ||
                         keyword_type == CosToken_Type_XRef ||
                         keyword_type == CosToken_Type_N ||
                         keyword_type == CosToken_Type_F ||
                         keyword_type == CosToken_Type_Trailer ||
                         keyword_type == CosToken_Type_StartXRef);

    const CosToken * const peeked_token = cos_tokenizer_peek_next_token(tokenizer,
                                                                        NULL);
    if (!peeked_token) {
        return false;
    }

    if (peeked_token->type == keyword_type) {
        // Consume the token.
        cos_consume_next_token_(tokenizer);

        return true;
    }

    return false;
}

static CosToken *
cos_tokenizer_read_next_token_(CosTokenizer *tokenizer,
                               CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    if (!tokenizer) {
        return NULL;
    }

    CosToken * const token = cos_acquire_token_(tokenizer);
    if (!token) {
        goto failure;
    }

    // Skip over ignorable whitespace characters and comments.
    cos_tokenizer_skip_whitespace_and_comments_(tokenizer);

    const int c = cos_input_stream_reader_getc(tokenizer->input_stream_reader);
    switch (c) {
        case EOF: {
            token->type = CosToken_Type_EOF;
        } break;

        case CosCharacterSet_LeftSquareBracket: {
            token->type = CosToken_Type_ArrayStart;
        } break;
        case CosCharacterSet_RightSquareBracket: {
            token->type = CosToken_Type_ArrayEnd;
        } break;

        case CosCharacterSet_Solidus: {
            // This is a name.
            CosString * const string = cos_string_alloc(0);
            if (!string) {
                // Error: out of memory.
                break;
            }

            CosError error;
            if (cos_tokenizer_read_name_(tokenizer, string, &error)) {
                token->type = CosToken_Type_Name;
                cos_token_value_set_string(token->value, string);
            }
            else {
                // Error: invalid name.
                token->type = CosToken_Type_Unknown;

                cos_error_propagate(out_error, error);

                cos_string_free(string);
            }
        } break;

        case CosCharacterSet_LeftParenthesis: {
            // This is a literal string.
            CosData * const data = cos_data_alloc(0);
            if (!data) {
                // Error: out of memory.
                break;
            }

            if (cos_tokenizer_read_literal_string_(tokenizer, data)) {
                token->type = CosToken_Type_Literal_String;
                cos_token_value_set_data(token->value, data);
            }
            else {
                // Error: invalid literal string.
                token->type = CosToken_Type_Unknown;

                cos_data_free(data);
            }
        } break;

        case CosCharacterSet_LessThanSign: {
            // This is either a dictionary ("<<") or a hex string ("<").
            if (cos_tokenizer_match_(tokenizer, CosCharacterSet_LessThanSign)) {
                token->type = CosToken_Type_DictionaryStart;
            }
            else {
                // This is a hex string.
                CosData * const data = cos_data_alloc(0);
                if (!data) {
                    // Error: out of memory.
                    break;
                }

                CosError error;
                if (cos_tokenizer_read_hex_string_(tokenizer, data, &error)) {
                    token->type = CosToken_Type_Hex_String;
                    cos_token_value_set_data(token->value, data);
                }
                else {
                    // Error: invalid hex string.
                    token->type = CosToken_Type_Unknown;

                    cos_error_propagate(out_error, error);

                    cos_data_free(data);
                }
            }
        } break;

        case CosCharacterSet_GreaterThanSign: {
            if (cos_tokenizer_match_(tokenizer, CosCharacterSet_GreaterThanSign)) {
                token->type = CosToken_Type_DictionaryEnd;
            }
            else {
                // Unexpected character.
            }
        } break;

        case CosCharacterSet_DigitZero:
        case CosCharacterSet_DigitOne:
        case CosCharacterSet_DigitTwo:
        case CosCharacterSet_DigitThree:
        case CosCharacterSet_DigitFour:
        case CosCharacterSet_DigitFive:
        case CosCharacterSet_DigitSix:
        case CosCharacterSet_DigitSeven:
        case CosCharacterSet_DigitEight:
        case CosCharacterSet_DigitNine:
        case CosCharacterSet_FullStop:
        case CosCharacterSet_PlusSign:
        case CosCharacterSet_HyphenMinus: {
            // We know that this is a number (integer or real).
            cos_input_stream_reader_ungetc(tokenizer->input_stream_reader);

            CosNumber number_value = {0};
            CosError error;
            if (cos_read_number_(tokenizer, &number_value, &error)) {
                // This is a number.
                if (number_value.type == CosNumberType_Integer) {
                    token->type = CosToken_Type_Integer;
                    cos_token_value_set_integer_number(token->value,
                                                       number_value.value.integer);
                }
                else {
                    token->type = CosToken_Type_Real;
                    cos_token_value_set_real_number(token->value,
                                                    number_value.value.real);
                }
            }
            else {
                // Error: invalid number.
                token->type = CosToken_Type_Unknown;

                cos_error_propagate(out_error, error);
            }
        } break;

        default: {
            cos_input_stream_reader_ungetc(tokenizer->input_stream_reader);

            // This could be a keyword or an unknown token.
            CosString *string = cos_string_alloc(0);
            if (!string) {
                // Error: out of memory.
                break;
            }

            CosError error;
            if (cos_tokenizer_read_token_(tokenizer, string, &error)) {
                // This might be a keyword.
                const CosToken_Type keyword_type = cos_keyword_token_type_from_string_(cos_string_get_ref(string));
                if (keyword_type != CosToken_Type_Unknown) {
                    // This is a keyword.
                    token->type = keyword_type;
                }
                else {
                    // Error: unrecognized token.
                    token->type = CosToken_Type_Unknown;
                }

                cos_string_free(string);
            }
            else {
                // Error: invalid token.
                token->type = CosToken_Type_Unknown;

                cos_error_propagate(out_error, error);

                cos_string_free(string);
            }
        } break;
    }

    return token;

failure:
    if (token) {
        cos_release_token_(tokenizer, token);
    }
    return NULL;
}

static inline int
cos_tokenizer_get_next_char_(CosTokenizer *tokenizer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    if (!tokenizer) {
        return EOF;
    }

    return cos_input_stream_reader_getc(tokenizer->input_stream_reader);
}

static inline int
cos_tokenizer_peek_next_char_(CosTokenizer *tokenizer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    if (!tokenizer) {
        return EOF;
    }

    return cos_input_stream_reader_peek(tokenizer->input_stream_reader);
}

static bool
cos_tokenizer_match_(CosTokenizer *tokenizer, char character)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    int c = cos_tokenizer_peek_next_char_(tokenizer);
    if (c == character) {
        cos_tokenizer_get_next_char_(tokenizer);

        return true;
    }
    else {
        return false;
    }
}

static char
cos_hex_digit_to_int_(int character)
{
    switch (character) {
        case CosCharacterSet_DigitZero:
            return 0;
        case CosCharacterSet_DigitOne:
            return 1;
        case CosCharacterSet_DigitTwo:
            return 2;
        case CosCharacterSet_DigitThree:
            return 3;
        case CosCharacterSet_DigitFour:
            return 4;
        case CosCharacterSet_DigitFive:
            return 5;
        case CosCharacterSet_DigitSix:
            return 6;
        case CosCharacterSet_DigitSeven:
            return 7;
        case CosCharacterSet_DigitEight:
            return 8;
        case CosCharacterSet_DigitNine:
            return 9;
        case CosCharacterSet_LatinCapitalLetterA:
        case CosCharacterSet_LatinSmallLetterA:
            return 10;
        case CosCharacterSet_LatinCapitalLetterB:
        case CosCharacterSet_LatinSmallLetterB:
            return 11;
        case CosCharacterSet_LatinCapitalLetterC:
        case CosCharacterSet_LatinSmallLetterC:
            return 12;
        case CosCharacterSet_LatinCapitalLetterD:
        case CosCharacterSet_LatinSmallLetterD:
            return 13;
        case CosCharacterSet_LatinCapitalLetterE:
        case CosCharacterSet_LatinSmallLetterE:
            return 14;
        case CosCharacterSet_LatinCapitalLetterF:
        case CosCharacterSet_LatinSmallLetterF:
            return 15;

        default:
            return 0;
    }
}

static int
cos_decimal_digit_to_int_(int character)
{
    switch (character) {
        case CosCharacterSet_DigitZero:
            return 0;
        case CosCharacterSet_DigitOne:
            return 1;
        case CosCharacterSet_DigitTwo:
            return 2;
        case CosCharacterSet_DigitThree:
            return 3;
        case CosCharacterSet_DigitFour:
            return 4;
        case CosCharacterSet_DigitFive:
            return 5;
        case CosCharacterSet_DigitSix:
            return 6;
        case CosCharacterSet_DigitSeven:
            return 7;
        case CosCharacterSet_DigitEight:
            return 8;
        case CosCharacterSet_DigitNine:
            return 9;

        default:
            return -1;
    }
}

static int
cos_octal_digit_to_int_(int character)
{
    switch (character) {
        case CosCharacterSet_DigitZero:
            return 0;
        case CosCharacterSet_DigitOne:
            return 1;
        case CosCharacterSet_DigitTwo:
            return 2;
        case CosCharacterSet_DigitThree:
            return 3;
        case CosCharacterSet_DigitFour:
            return 4;
        case CosCharacterSet_DigitFive:
            return 5;
        case CosCharacterSet_DigitSix:
            return 6;
        case CosCharacterSet_DigitSeven:
            return 7;

        default:
            return 0;
    }
}

static bool
cos_tokenizer_read_name_(CosTokenizer *tokenizer,
                         CosString *string,
                         CosError *error)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    COS_PARAMETER_ASSERT(string != NULL);

    int c;
    while ((c = cos_tokenizer_get_next_char_(tokenizer)) != EOF) {
        if (cos_is_whitespace(c) || cos_is_delimiter(c)) {
            // This is the end of the name.
            cos_input_stream_reader_ungetc(tokenizer->input_stream_reader);
            return true;
        }
        else if (c == CosCharacterSet_NumberSign) {
            // Beginning of a hexadecimal escape sequence.
            int hex_value = cos_tokenizer_handle_name_hex_escape_sequence_(tokenizer);
            if (hex_value < 0) {
                // Error: invalid hexadecimal escape sequence.
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_SYNTAX,
                                                   "Invalid hexadecimal escape sequence"),
                                    error);
                return false;
            }
            cos_string_push_back(string, (char)hex_value);
        }
        else {
            cos_string_push_back(string, (char)c);
        }
    }

    return true;
}

static int
cos_tokenizer_handle_name_hex_escape_sequence_(CosTokenizer *tokenizer)
{
    int hex_value = 0;

    for (int i = 0; i < 2; i++) {
        const int character = cos_tokenizer_peek_next_char_(tokenizer);
        if (COS_LIKELY(cos_is_hex_digit(character))) {
            cos_tokenizer_get_next_char_(tokenizer);

            const char hex_digit = cos_hex_digit_to_int_(character);

            // Fill in the hex value from the most significant digit to the least significant digit.
            hex_value = (hex_value << 4) | hex_digit;
        }
        else {
            // Error: invalid hexadecimal escape sequence.
            return -1;
        }
    }

    return hex_value;
}

static bool
cos_tokenizer_read_literal_string_(CosTokenizer *tokenizer,
                                   CosData *data)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    COS_PARAMETER_ASSERT(data != NULL);

    unsigned int nested_parentheses_level = 0;

    int c = EOF;
    while ((c = cos_tokenizer_get_next_char_(tokenizer)) != EOF) {
        switch (c) {
            case CosCharacterSet_LeftParenthesis: {
                COS_ASSERT(nested_parentheses_level < UINT_MAX,
                           "Maximum number of parentheses reached.");

                nested_parentheses_level++;
            } break;
            case CosCharacterSet_RightParenthesis: {
                if (nested_parentheses_level > 0) {
                    nested_parentheses_level--;
                }
                else {
                    // This is the end of the literal string.
                    return true;
                }
            } break;

            case CosCharacterSet_ReverseSolidus: {
                // Beginning of an escape sequence.
                unsigned char escaped_character = 0;
                if (cos_tokenizer_handle_literal_string_escape_sequence_(tokenizer,
                                                                         &escaped_character)) {
                    c = escaped_character;
                }
                else {
                    // The escape sequence did not produce a character.
                    goto continue_label;
                }
            } break;

            case CosCharacterSet_CarriageReturn: {
                /*
                 * An end-of-line marker appearing within a literal string without a preceding REVERSE SOLIDUS
                 * shall be treated as a byte value of (0Ah), irrespective of whether the end-of-line marker was
                 * a CARRIAGE RETURN (0Dh), a LINE FEED (0Ah), or both.
                 */
                if (cos_tokenizer_match_(tokenizer, CosCharacterSet_LineFeed)) {
                    // Treating the CRLF as LF.
                }
                c = CosCharacterSet_LineFeed;
            } break;

            default:
                break;
        }

        cos_data_push_back(data, (unsigned char)c, NULL);

    continue_label:;
    }

    // Error: unterminated literal string.
    return false;
}

static bool
cos_tokenizer_handle_literal_string_escape_sequence_(CosTokenizer *tokenizer, unsigned char *result)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    const int c = cos_tokenizer_get_next_char_(tokenizer);
    switch (c) {
        case EOF: {
            // This is an error.
            // Expected a character after the reverse solidus.
            return false;
        }

        case 'n': {
            if (result) {
                *result = CosCharacterSet_LineFeed;
            }
        } break;
        case 'r': {
            if (result) {
                *result = CosCharacterSet_CarriageReturn;
            }
        } break;
        case 't': {
            if (result) {
                *result = CosCharacterSet_HorizontalTab;
            }
        } break;
        case 'b': {
            if (result) {
                *result = CosCharacterSet_Backspace;
            }
        } break;
        case 'f': {
            if (result) {
                *result = CosCharacterSet_FormFeed;
            }
        } break;

        case CosCharacterSet_LeftParenthesis:
        case CosCharacterSet_RightParenthesis:
        case CosCharacterSet_ReverseSolidus: {
            // Escaped characters that are replaced with themselves.
            if (result) {
                *result = (unsigned char)c;
            }
        } break;

        case CosCharacterSet_DigitZero:
        case CosCharacterSet_DigitOne:
        case CosCharacterSet_DigitTwo:
        case CosCharacterSet_DigitThree:
        case CosCharacterSet_DigitFour:
        case CosCharacterSet_DigitFive:
        case CosCharacterSet_DigitSix:
        case CosCharacterSet_DigitSeven: {
            // Octal escape sequence.
            // Put the octal digit back and read the octal escape sequence.
            cos_input_stream_reader_ungetc(tokenizer->input_stream_reader);

            const int escaped_value = cos_tokenizer_read_octal_escape_sequence_(tokenizer);
            if (result) {
                *result = (unsigned char)escaped_value;
            }
        } break;

        case CosCharacterSet_CarriageReturn:
            if (cos_tokenizer_match_(tokenizer, CosCharacterSet_LineFeed)) {
                // Treating the CRLF as LF.
            }
            COS_ATTR_FALLTHROUGH;
        case CosCharacterSet_LineFeed: {
            // This is a line continuation.
            // The reverse solidus and the end-of-line marker are discarded.
            return false;
        }

        default: {
            // Unknown escape sequence character.
            // Ignore the reverse solidus and put the character back.
            cos_input_stream_reader_ungetc(tokenizer->input_stream_reader);
            return false;
        }
    }

    return true;
}

static int
cos_tokenizer_read_octal_escape_sequence_(CosTokenizer *tokenizer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    if (!tokenizer) {
        return 0;
    }

    int value = 0;

    for (int i = 0; i < 3; i++) {
        const int c = cos_tokenizer_get_next_char_(tokenizer);
        if (cos_is_octal_digit(c)) {
            const int octal_digit_value = cos_octal_digit_to_int_(c);

            // Fill in the octal value from the most significant digit to the least significant digit.
            value = (value << 3) | octal_digit_value;
        }
        else {
            // This is the end of the octal escape sequence.
            COS_ASSERT(i > 0, "Expected at least one octal digit.");
            cos_input_stream_reader_ungetc(tokenizer->input_stream_reader);
            break;
        }
    }

    // Ignore high-order overflow.
    return (unsigned char)value;
}

static bool
cos_tokenizer_read_hex_string_(CosTokenizer *tokenizer,
                               CosData *data,
                               CosError *error)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    COS_PARAMETER_ASSERT(data != NULL);

    int hex_value = 0;
    bool odd_number_of_hex_digits = false;

    int character = EOF;
    while ((character = cos_tokenizer_get_next_char_(tokenizer)) != EOF) {
        if (COS_LIKELY(cos_is_hex_digit(character))) {
            // This is a hex digit.
            const char hex_digit_value = cos_hex_digit_to_int_(character);
            if (odd_number_of_hex_digits) {
                // This is the second hex digit of a byte.
                // Fill in the hex value from the most significant digit to the least significant digit.
                hex_value = (hex_value << 4) | hex_digit_value;

                // Write the byte to the buffer.
                cos_data_push_back(data, (unsigned char)hex_value, NULL);

                // Reset the hex value.
                hex_value = 0;
            }
            else {
                // This is the first hex digit of a byte (ie most significant digit).
                hex_value = (unsigned char)hex_digit_value;
            }
            odd_number_of_hex_digits = !odd_number_of_hex_digits;
        }
        else if (character == CosCharacterSet_GreaterThanSign) {
            // This is the end of the hex string.
            if (odd_number_of_hex_digits) {
                // Write the last byte to the buffer.
                hex_value = (hex_value << 4);
                cos_data_push_back(data, (unsigned char)hex_value, NULL);
            }
            return true;
        }
        else {
            // Skip all non-hex characters, including whitespace.
            continue;
        }
    }

    // Error: unterminated hex string.
    if (error) {
        *error = cos_error_make(COS_ERROR_SYNTAX,
                                "Unterminated hex string");
    }
    return false;
}

static bool
cos_read_number_(CosTokenizer *tokenizer,
                 CosNumber *out_number,
                 CosError *out_error)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    COS_PARAMETER_ASSERT(out_number != NULL);
    if (!tokenizer || !out_number) {
        return false;
    }

    bool has_sign = false;
    bool has_decimal_point = false;
    unsigned int digit_count = 0;
    unsigned int fractional_digit_count = 0;
    double fractional_scale = 0.1;

    int int_value = 0;
    double real_value = 0.0;

    /*
     * A number is an optional sign and a sequence of digits, with an optional decimal point.
     */

    int c = EOF;
    while ((c = cos_tokenizer_get_next_char_(tokenizer)) != EOF) {
        const int digit_value = cos_decimal_digit_to_int_(c);
        if (digit_value >= 0) {
            digit_count++;

            if (has_decimal_point) {
                fractional_digit_count++;
                real_value = real_value + (digit_value * fractional_scale);
                fractional_scale *= 0.1;
            }
            else {
                int_value = (int_value * 10) + digit_value;
            }
        }
        else if ((c == CosCharacterSet_PlusSign || c == CosCharacterSet_HyphenMinus) &&
                 !(has_sign || has_decimal_point || digit_count > 0)) {
            has_sign = true;
        }
        else if (c == CosCharacterSet_FullStop && !has_decimal_point) {
            // Switch to reading the fractional part of a real number.
            has_decimal_point = true;
            real_value = (double)int_value;
        }
        else {
            // This is the end of the number.
            cos_input_stream_reader_ungetc(tokenizer->input_stream_reader);
            break;
        }
    }

    if (digit_count == 0) {
        // Error: invalid number.
        if (out_error) {
            *out_error = cos_error_make(COS_ERROR_SYNTAX, "Invalid number: no digits");
        }
        return false;
    }

    if (tokenizer->strict && fractional_digit_count > COS_REAL_MAX_SIG_FRAC_DIG) {
        if (out_error) {
            *out_error = cos_error_make(COS_ERROR_SYNTAX,
                                        "Too many fractional digits");
        }
        return false;
    }

    if (has_decimal_point) {
        *out_number = cos_number_make_real(real_value);
    }
    else {
        *out_number = cos_number_make_integer(int_value);
    }

    return true;
}

static void
cos_tokenizer_skip_whitespace_and_comments_(CosTokenizer *tokenizer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    int c = EOF;
    while ((c = cos_tokenizer_get_next_char_(tokenizer)) != EOF) {
        if (cos_is_whitespace(c)) {
            continue;
        }
        else if (c == CosCharacterSet_PercentSign) {
            // Skip comment.
            cos_tokenizer_skip_comment_(tokenizer);
        }
        else {
            // This is not a whitespace character.
            cos_input_stream_reader_ungetc(tokenizer->input_stream_reader);
            break;
        }
    }
}

static void
cos_tokenizer_skip_character_(CosTokenizer *tokenizer,
                              int character)
{
    const int c = cos_tokenizer_get_next_char_(tokenizer);
    if (c != EOF && c != character) {
        cos_input_stream_reader_ungetc(tokenizer->input_stream_reader);
    }
}

static void
cos_tokenizer_skip_comment_(CosTokenizer *tokenizer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    int c = EOF;
    while ((c = cos_tokenizer_get_next_char_(tokenizer)) != EOF) {
        if (cos_is_end_of_line(c)) {
            // This is the end of the comment.
            if (c == CosCharacterSet_CarriageReturn) {
                cos_tokenizer_skip_character_(tokenizer,
                                              CosCharacterSet_LineFeed);
            }
            return;
        }
        // Skip regular character in comment.
    }
}

static bool
cos_tokenizer_read_token_(CosTokenizer *tokenizer, CosString *string, CosError *error)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    COS_PARAMETER_ASSERT(string != NULL);

    int c = EOF;
    while ((c = cos_tokenizer_get_next_char_(tokenizer)) != EOF) {
        if (cos_is_whitespace(c) || cos_is_delimiter(c)) {
            // This is the end of the token.
            cos_input_stream_reader_ungetc(tokenizer->input_stream_reader);
            return true;
        }

        if (!cos_string_push_back(string, (char)c)) {
            // Error: out of memory.
            if (error) {
                *error = cos_error_make(COS_ERROR_MEMORY, "Out of memory");
            }
            return false;
        }
    }

    return true;
}

static CosToken_Type
cos_keyword_token_type_from_string_(CosStringRef string)
{
    // The longest keywords are 9 characters long.
    if (string.length == 0 || string.length > 9) {
        return CosToken_Type_Unknown;
    }

    switch (string.data[0]) {
        case 't': {
            if (cos_string_ref_cmp(string, cos_string_ref_const("true")) == 0) {
                return CosToken_Type_True;
            }
            else if (cos_string_ref_cmp(string, cos_string_ref_const("trailer")) == 0) {
                return CosToken_Type_Trailer;
            }
        } break;
        case 'f': {
            if (cos_string_ref_cmp(string, cos_string_ref_const("f")) == 0) {
                return CosToken_Type_F;
            }
            else if (cos_string_ref_cmp(string, cos_string_ref_const("false")) == 0) {
                return CosToken_Type_False;
            }
        } break;
        case 'n': {
            if (cos_string_ref_cmp(string, cos_string_ref_const("n")) == 0) {
                return CosToken_Type_N;
            }
            else if (cos_string_ref_cmp(string, cos_string_ref_const("null")) == 0) {
                return CosToken_Type_Null;
            }
        } break;
        case 'R': {
            if (string.length == 1) {
                return CosToken_Type_R;
            }
        } break;
        case 'o': {
            if (cos_string_ref_cmp(string, cos_string_ref_const("obj")) == 0) {
                return CosToken_Type_Obj;
            }
        } break;
        case 'e': {
            if (cos_string_ref_cmp(string, cos_string_ref_const("endobj")) == 0) {
                return CosToken_Type_EndObj;
            }
            else if (cos_string_ref_cmp(string, cos_string_ref_const("endstream")) == 0) {
                return CosToken_Type_EndStream;
            }
        } break;
        case 's': {
            if (cos_string_ref_cmp(string, cos_string_ref_const("stream")) == 0) {
                return CosToken_Type_Stream;
            }
            else if (cos_string_ref_cmp(string, cos_string_ref_const("startxref")) == 0) {
                return CosToken_Type_StartXRef;
            }
        } break;
        case 'x': {
            if (cos_string_ref_cmp(string, cos_string_ref_const("xref")) == 0) {
                return CosToken_Type_XRef;
            }
        } break;

        default:
            break;
    }

    return CosToken_Type_Unknown;
}

COS_ASSUME_NONNULL_END
