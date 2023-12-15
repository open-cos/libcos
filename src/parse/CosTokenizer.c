//
// Created by david on 03/06/23.
//

#include "CosTokenizer.h"

#include "common/Assert.h"
#include "common/CharacterSet.h"
#include "common/CosDataBuffer.h"
#include "common/CosString.h"
#include "common/CosVector.h"
#include "io/CosInputStreamReader.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CosToken.h"

COS_ASSUME_NONNULL_BEGIN

struct CosTokenizer {
    CosInputStreamReader *input_stream_reader;

    CosToken * COS_Nullable peeked_token;

    CosToken current_token;

    CosDataBuffer *text_buffer;
};

static inline bool
cos_tokenizer_is_at_end_(CosTokenizer *tokenizer);

static inline int
cos_tokenizer_get_next_char(CosTokenizer *tokenizer);

static inline int
cos_tokenizer_peek_next_char(CosTokenizer *tokenizer);

/**
 * Advances the tokenizer if the next character matches the given character.
 *
 * @param tokenizer The tokenizer.
 * @param character The character to match.
 *
 * @return true if the next character matches the given character, false otherwise.
 */
static bool
cos_tokenizer_match(CosTokenizer *tokenizer, char character);

static void
cos_tokenizer_skip_whitespace_and_comments_(CosTokenizer *tokenizer);

static bool
cos_tokenizer_skip_whitespace_character_(CosTokenizer *tokenizer);

static void
cos_tokenizer_skip_comment_(CosTokenizer *tokenizer);

static bool
cos_tokenizer_read_name_(CosTokenizer *tokenizer, CosDataBuffer *buffer);

static int
cos_tokenizer_handle_name_hex_escape_sequence_(CosTokenizer *tokenizer);

static bool
cos_tokenizer_read_literal_string_(CosTokenizer *tokenizer, CosDataBuffer *buffer);

/**
 * Handles an escape sequence in a literal string.
 *
 * @param tokenizer The tokenizer.
 * @param result The output parameter for the escaped character.
 *
 * @return @c true if a character was written to @p result, @c false otherwise.
 */
static bool
cos_tokenizer_handle_literal_string_escape_sequence_(CosTokenizer *tokenizer, unsigned char *result)
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
cos_tokenizer_read_hex_string_(CosTokenizer *tokenizer, CosDataBuffer *buffer, CosError **error);

static bool
cos_tokenizer_read_number_(CosTokenizer *tokenizer,
                           int *int_result,
                           double *double_result,
                           CosError **error);

static bool
cos_tokenizer_read_token_(CosTokenizer *tokenizer, CosString *string, CosError *error);

static inline bool
cos_check_keyword_(CosStringRef text, CosToken_Type *type);

CosTokenizer *
cos_tokenizer_alloc(CosInputStream *input_stream)
{
    CosTokenizer *tokenizer = NULL;
    CosInputStreamReader *input_stream_reader = NULL;

    tokenizer = malloc(sizeof(CosTokenizer));
    if (!tokenizer) {
        goto failure;
    }

    input_stream_reader = cos_input_stream_reader_alloc(input_stream);
    if (!input_stream_reader) {
        goto failure;
    }

    tokenizer->input_stream_reader = input_stream_reader;

    tokenizer->peeked_token = NULL;

    tokenizer->text_buffer = cos_data_buffer_alloc();
    if (!tokenizer->text_buffer) {
        goto failure;
    }

    return tokenizer;

failure:
    if (input_stream_reader) {
        cos_input_stream_reader_free(input_stream_reader);
    }
    if (tokenizer) {
        free(tokenizer);
    }

    return NULL;
}

void
cos_tokenizer_free(CosTokenizer *tokenizer)
{
    if (!tokenizer) {
        return;
    }

    cos_input_stream_reader_free(tokenizer->input_stream_reader);

    cos_data_buffer_free(tokenizer->text_buffer);

    free(tokenizer);
}

CosToken *
cos_tokenizer_next_token(CosTokenizer *tokenizer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    if (!tokenizer) {
        return NULL;
    }

    // If we have a peeked token, return it.
    if (tokenizer->peeked_token) {
        CosToken * const token = tokenizer->peeked_token;
        tokenizer->peeked_token = NULL;
        return token;
    }

    CosToken *token = malloc(sizeof(CosToken));
    if (!token) {
        return NULL;
    }

    token->type = CosToken_Type_Unknown;
    token->value = cos_token_value_none();

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
            CosDataBuffer * const buffer = cos_data_buffer_alloc();
            if (cos_tokenizer_read_name_(tokenizer, buffer)) {
                token->type = CosToken_Type_Name;
                token->value = cos_token_value_data(NULL);
            }
            else {
                // Error: invalid name.
                token->type = CosToken_Type_Unknown;

                cos_data_buffer_free(buffer);
            }
        } break;

        case CosCharacterSet_LeftParenthesis: {
            // This is a literal string.
            CosDataBuffer * const buffer = cos_data_buffer_alloc();
            if (cos_tokenizer_read_literal_string_(tokenizer, buffer)) {
                token->type = CosToken_Type_Literal_String;
                token->value = cos_token_value_data(NULL);
            }
            else {
                // Error: unterminated literal string.
                token->type = CosToken_Type_Unknown;

                cos_data_buffer_free(buffer);
            }
        } break;

        case CosCharacterSet_LessThanSign: {
            // This is either a dictionary ("<<") or a hex string ("<").
            if (cos_tokenizer_match(tokenizer, CosCharacterSet_LessThanSign)) {
                token->type = CosToken_Type_DictionaryStart;
            }
            else {
                // This is a hex string.
                CosDataBuffer * const buffer = cos_data_buffer_alloc();
                if (cos_tokenizer_read_hex_string_(tokenizer, buffer, NULL)) {
                    token->type = CosToken_Type_Hex_String;
                    token->value = cos_token_value_data(NULL);
                }
                else {
                    // Error: unterminated hex string.
                    token->type = CosToken_Type_Unknown;

                    cos_data_buffer_free(buffer);
                }
            }
        } break;

        case CosCharacterSet_GreaterThanSign: {
            if (cos_tokenizer_match(tokenizer, CosCharacterSet_GreaterThanSign)) {
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
        case CosCharacterSet_FullStop: {
            cos_input_stream_reader_ungetc(tokenizer->input_stream_reader);

            // We know that this is a number (integer or real).
            cos_tokenizer_read_number_(tokenizer, NULL, NULL, NULL);
        } break;
        case CosCharacterSet_PlusSign:
        case CosCharacterSet_HyphenMinus: {
            // We know that this is a number (integer or real).
            cos_tokenizer_read_number_(tokenizer, NULL, NULL, NULL);
        } break;

        default: {
            // This could be a keyword or an unknown token.
            CosString *string = cos_string_alloc(0);
            if (!string) {
                // Error: out of memory.
                break;
            }

            CosError error;
            if (cos_tokenizer_read_token_(tokenizer, string, &error)) {
                // This might be a keyword.
                CosToken_Type type = CosToken_Type_Unknown;
                if (cos_check_keyword_(cos_string_make_ref(string), &type)) {
                    // This is a keyword.
                    token->type = type;

                    cos_string_free(string);
                }
                else {
                    // Error: unrecognized token.
                    token->type = CosToken_Type_Unknown;
                    token->value = cos_token_value_string(string);
                }
            }
            else {
                // Error: invalid token.
                token->type = CosToken_Type_Unknown;

                cos_string_free(string);
            }
        } break;
    }

    return token;
}

CosToken *
cos_tokenizer_peek_token(CosTokenizer *tokenizer)
{
    if (!tokenizer->peeked_token) {
        tokenizer->peeked_token = cos_tokenizer_next_token(tokenizer);
    }

    return tokenizer->peeked_token;
}

static inline bool
cos_tokenizer_is_at_end_(CosTokenizer *tokenizer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    return cos_input_stream_reader_is_at_end(tokenizer->input_stream_reader);
}

static inline int
cos_tokenizer_get_next_char(CosTokenizer *tokenizer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    return cos_input_stream_reader_getc(tokenizer->input_stream_reader);
}

static inline int
cos_tokenizer_peek_next_char(CosTokenizer *tokenizer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    return cos_input_stream_reader_peek(tokenizer->input_stream_reader);
}

static CosToken
cos_tokenizer_make_token(CosTokenizer *tokenizer, CosToken_Type type, CosTokenValue *value)
{

    const CosToken token = {
        .type = type,
        .text = {0},
        //        .value = value,
    };
    return token;
}

static bool
cos_tokenizer_match(CosTokenizer *tokenizer, char character)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    int c = cos_tokenizer_peek_next_char(tokenizer);
    if (c == character) {
        cos_tokenizer_get_next_char(tokenizer);

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
cos_tokenizer_read_name_(CosTokenizer *tokenizer, CosDataBuffer *buffer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    int character;
    while ((character = cos_tokenizer_get_next_char(tokenizer)) != EOF) {
        switch (character) {
            default: {
                // This is a regular character.
            } break;

            case CosCharacterSet_NumberSign: {
                // Beginning of a hexadecimal escape sequence.
                int hex_value = cos_tokenizer_handle_name_hex_escape_sequence_(tokenizer);
                if (hex_value < 0) {
                    // Error: invalid hexadecimal escape sequence.
                    return false;
                }
            } break;

            case CosCharacterSet_Nul:
            case CosCharacterSet_HorizontalTab:
            case CosCharacterSet_LineFeed:
            case CosCharacterSet_FormFeed:
            case CosCharacterSet_CarriageReturn:
            case CosCharacterSet_Space:
            case CosCharacterSet_ReverseSolidus: {
                // This is the end of the name.
                cos_input_stream_reader_ungetc(tokenizer->input_stream_reader);
                return true;
            }
        }

        cos_data_buffer_push_back(buffer, (unsigned char)character, NULL);
    }

    return true;
}

static int
cos_tokenizer_handle_name_hex_escape_sequence_(CosTokenizer *tokenizer)
{
    int hex_value = 0;

    for (int i = 0; i < 2; i++) {
        const int character = cos_tokenizer_peek_next_char(tokenizer);
        if (COS_LIKELY(cos_is_hex_digit(character))) {
            cos_tokenizer_get_next_char(tokenizer);

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
cos_tokenizer_read_literal_string_(CosTokenizer *tokenizer, CosDataBuffer *buffer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    unsigned int nested_parentheses_level = 0;

    int c = EOF;
    while ((c = cos_tokenizer_get_next_char(tokenizer)) != EOF) {
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
                if (cos_tokenizer_match(tokenizer, CosCharacterSet_LineFeed)) {
                    // Treating the CRLF as LF.
                }
                c = CosCharacterSet_LineFeed;
            } break;

            default:
                break;
        }

        cos_data_buffer_push_back(buffer, (unsigned char)c, NULL);

    continue_label:;
    }

    // Error: unterminated literal string.
    return false;
}

static bool
cos_tokenizer_handle_literal_string_escape_sequence_(CosTokenizer *tokenizer, unsigned char *result)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    const int c = cos_tokenizer_get_next_char(tokenizer);
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
            if (cos_tokenizer_match(tokenizer, CosCharacterSet_LineFeed)) {
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

    int value = 0;

    for (int i = 0; i < 3; i++) {
        const int c = cos_tokenizer_get_next_char(tokenizer);
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
cos_tokenizer_read_hex_string_(CosTokenizer *tokenizer, CosDataBuffer *buffer, CosError **error)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    COS_PARAMETER_ASSERT(buffer != NULL);

    int hex_value = 0;
    bool odd_number_of_hex_digits = false;

    int character = EOF;
    while ((character = cos_tokenizer_get_next_char(tokenizer)) != EOF) {
        if (COS_LIKELY(cos_is_hex_digit(character))) {
            // This is a hex digit.
            const char hex_digit_value = cos_hex_digit_to_int_(character);
            if (odd_number_of_hex_digits) {
                // This is the second hex digit of a byte.
                // Fill in the hex value from the most significant digit to the least significant digit.
                hex_value = (hex_value << 4) | hex_digit_value;

                // Write the byte to the buffer.
                cos_data_buffer_push_back(buffer, (unsigned char)hex_value, NULL);

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
                cos_data_buffer_push_back(buffer, (unsigned char)hex_value, NULL);
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
        *error = cos_error_alloc(COS_ERROR_SYNTAX, "Unterminated hex string");
    }
    return false;
}

static bool
cos_tokenizer_read_number_(CosTokenizer *tokenizer,
                           int *int_result,
                           double *double_result,
                           CosError **error)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    typedef enum CosNumberType {
        CosNumberType_Integer,
        CosNumberType_Real,
    } CosNumberType;

    typedef struct CosNumberState {
        CosNumberType type;
        unsigned int digit_count;
    } CosNumberState;

    CosNumberState state = {
        .type = CosNumberType_Integer,
        .digit_count = 0,
    };

    CosDataBuffer * const buffer = cos_data_buffer_alloc();

    int character = 0;
    while ((character = cos_tokenizer_get_next_char(tokenizer)) != EOF) {
        switch (character) {

            case CosCharacterSet_FullStop: {
                state.type = CosNumberType_Real;
            } break;

            default: {
                if (COS_UNLIKELY(!cos_is_decimal_digit(character))) {
                    // This is the end of the number.
                    cos_input_stream_reader_ungetc(tokenizer->input_stream_reader);
                    goto end_label;
                }
                state.digit_count++;
            } break;
        }

        cos_data_buffer_push_back(buffer, (unsigned char)character, NULL);
        continue;

    end_label:
        break;
    }

    if (state.digit_count == 0) {
        // Error: invalid number.
        cos_data_buffer_free(buffer);
        return false;
    }

    CosString * const number_string = cos_data_buffer_to_string(buffer);
    if (!number_string) {
        // Error: out of memory.
        cos_data_buffer_free(buffer);
        return false;
    }

    cos_data_buffer_free(buffer);

    if (state.type == CosNumberType_Integer) {
        int integer_value = atoi(number_string->data);
    }
    else {
        double real_value = atof(number_string->data);
        return (real_value > 0);
    }

    cos_string_free(number_string);

    return true;
}

static void
cos_tokenizer_skip_whitespace_and_comments_(CosTokenizer *tokenizer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    while (cos_tokenizer_skip_whitespace_character_(tokenizer)) {
        // Skipped whitespace.
    }
}

static bool
cos_tokenizer_skip_whitespace_character_(CosTokenizer *tokenizer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    if (cos_tokenizer_is_at_end_(tokenizer)) {
        return false;
    }

    int c = cos_tokenizer_get_next_char(tokenizer);
    if (cos_is_whitespace(c)) {
        return true;
    }
    else if (c == CosCharacterSet_PercentSign) {
        // Skip comment.
        cos_tokenizer_skip_comment_(tokenizer);
        return true;
    }
    else {
        // This is not a whitespace character.
        cos_input_stream_reader_ungetc(tokenizer->input_stream_reader);
        return false;
    }
}

static void
cos_tokenizer_skip_character_(CosTokenizer *tokenizer, int character)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    int c = EOF;
    if ((c = cos_tokenizer_get_next_char(tokenizer)) != EOF) {
        if (c != character) {
            cos_input_stream_reader_ungetc(tokenizer->input_stream_reader);
        }
    }
}

static void
cos_tokenizer_skip_comment_(CosTokenizer *tokenizer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    int c = EOF;
    while ((c = cos_tokenizer_get_next_char(tokenizer)) != EOF) {
        if (cos_is_end_of_line(c)) {
            // This is the end of the comment.
            if (c == CosCharacterSet_CarriageReturn) {
                cos_tokenizer_skip_character_(tokenizer, CosCharacterSet_LineFeed);
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
    while ((c = cos_tokenizer_get_next_char(tokenizer)) != EOF) {
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

static inline bool
cos_check_keyword_(CosStringRef text, CosToken_Type *type)
{
    COS_PARAMETER_ASSERT(type != NULL);

    if (text.length == 0) {
        return false;
    }

    switch (text.data[0]) {
        case 't': {
            if (cos_string_ref_cmp(text, cos_string_ref_const("true")) == 0) {
                *type = CosToken_Type_Keyword_True;
                return true;
            }
            else if (cos_string_ref_cmp(text, cos_string_ref_const("trailer")) == 0) {
                *type = CosToken_Type_Keyword_Trailer;
                return true;
            }
        } break;
        case 'f': {
            if (cos_string_ref_cmp(text, cos_string_ref_const("false")) == 0) {
                *type = CosToken_Type_Keyword_False;
                return true;
            }
        } break;
        case 'n': {
            if (cos_string_ref_cmp(text, cos_string_ref_const("null")) == 0) {
                *type = CosToken_Type_Keyword_Null;
                return true;
            }
        } break;
        case 'R': {
            if (text.length == 1) {
                *type = CosToken_Type_Keyword_R;
                return true;
            }
        } break;
        case 'o': {
            if (cos_string_ref_cmp(text, cos_string_ref_const("obj")) == 0) {
                *type = CosToken_Type_Keyword_Obj;
                return true;
            }
        } break;
        case 'e': {
            if (cos_string_ref_cmp(text, cos_string_ref_const("endobj")) == 0) {
                *type = CosToken_Type_Keyword_EndObj;
                return true;
            }
            else if (cos_string_ref_cmp(text, cos_string_ref_const("endstream")) == 0) {
                *type = CosToken_Type_Keyword_EndStream;
                return true;
            }
        } break;
        case 's': {
            if (cos_string_ref_cmp(text, cos_string_ref_const("stream")) == 0) {
                *type = CosToken_Type_Keyword_Stream;
                return true;
            }
            else if (cos_string_ref_cmp(text, cos_string_ref_const("startxref")) == 0) {
                *type = CosToken_Type_Keyword_StartXRef;
                return true;
            }
        } break;
        case 'x': {
            if (cos_string_ref_cmp(text, cos_string_ref_const("xref")) == 0) {
                *type = CosToken_Type_Keyword_XRef;
                return true;
            }
        } break;

        default:
            break;
    }

    return false;
}

COS_ASSUME_NONNULL_END
