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
#include "libcos/io/CosInputStream.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "CosToken.h"

COS_ASSUME_NONNULL_BEGIN

struct CosTokenizer {
    CosInputStreamReader *input_stream_reader;

    CosToken *peeked_token;

    CosToken current_token;

    CosDataBuffer *text_buffer;
};

static int
cos_tokenizer_get_next_char(CosTokenizer *tokenizer);

static int
cos_tokenizer_peek_next_char(CosTokenizer *tokenizer);

static bool
cos_tokenizer_accept(CosTokenizer *tokenizer,
                     char character);

/**
 * Advances the tokenizer if the next character matches the given character.
 *
 * @param tokenizer The tokenizer.
 * @param character The character to match.
 *
 * @return true if the next character matches the given character, false otherwise.
 */
static bool
cos_tokenizer_match(CosTokenizer *tokenizer,
                    char character);

static bool
cos_tokenizer_read_name_(CosTokenizer *tokenizer,
                         CosDataBuffer *buffer);

static int
cos_tokenizer_handle_name_hex_escape_sequence_(CosTokenizer *tokenizer);

static bool
cos_tokenizer_read_literal_string_(CosTokenizer *tokenizer,
                                   CosDataBuffer *buffer);

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
cos_tokenizer_read_octal_escape_sequence_(CosTokenizer *
                                              tokenizer)
    COS_ATTR_ACCESS_WRITE_ONLY(2);

static bool
cos_tokenizer_read_hex_string_(CosTokenizer *tokenizer,
                               CosDataBuffer *buffer,
                               CosError **error);

static char *
cos_tokenizer_read_number(CosTokenizer *tokenizer);

static char *
cos_tokenizer_read_real(CosTokenizer *tokenizer);

static CosString *
cos_tokenizer_read_comment(CosTokenizer *tokenizer);

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
    //    token->value = NULL;

    int c = cos_input_stream_reader_getc(tokenizer->input_stream_reader);
    switch (c) {
        case CosCharacterSet_LeftSquareBracket: {
            token->type = CosToken_Type_ArrayStart;
        } break;
        case CosCharacterSet_RightSquareBracket: {
            token->type = CosToken_Type_ArrayEnd;
        } break;

        case CosCharacterSet_Solidus: {
            // This is a name.
            CosDataBuffer * const buffer = cos_data_buffer_alloc();
            if (cos_tokenizer_read_name_(tokenizer,
                                         buffer)) {
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
            CosDataBuffer * const buffer = cos_data_buffer_alloc();
            if (cos_tokenizer_read_literal_string_(tokenizer,
                                                   buffer)) {
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
                token->type = CosToken_Type_Hex_String;
                //                token->value = cos_tokenizer_read_hex_string_(tokenizer);
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

        case CosCharacterSet_PercentSign: {
            token->type = CosToken_Type_Comment;
            //            token->value = cos_tokenizer_read_comment(tokenizer);
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
        case CosCharacterSet_PlusSign:
        case CosCharacterSet_HyphenMinus: {
            // We know that this is a number (integer or real).
            cos_tokenizer_read_number(tokenizer);
        } break;

        case CosCharacterSet_FullStop: {
            // We know that this is a real number.
            token->type = CosToken_Type_Real;
            cos_tokenizer_read_real(tokenizer);
        } break;

        case EOF: {
            token->type = CosToken_Type_EOF;
        } break;

        case 't': {
            // "true" or "trailer".
            if (cos_tokenizer_match(tokenizer, 'r')) {
                if (cos_tokenizer_match(tokenizer, 'u') &&
                    cos_tokenizer_match(tokenizer, 'e')) {
                    token->type = CosToken_Type_Keyword_True;
                }
                else if (cos_tokenizer_match(tokenizer, 'a') &&
                         cos_tokenizer_match(tokenizer, 'i') &&
                         cos_tokenizer_match(tokenizer, 'l') &&
                         cos_tokenizer_match(tokenizer, 'e') &&
                         cos_tokenizer_match(tokenizer, 'r')) {
                    token->type = CosToken_Type_Keyword_Trailer;
                }
                else {
                    // Unexpected character.
                }
            }
            else {
                // Unexpected character.
            }
        } break;

        case 'f': {
            // "false".
            if (cos_tokenizer_match(tokenizer, 'a') &&
                cos_tokenizer_match(tokenizer, 'l') &&
                cos_tokenizer_match(tokenizer, 's') &&
                cos_tokenizer_match(tokenizer, 'e')) {
                token->type = CosToken_Type_Keyword_False;
            }
            else {
                // Unexpected character.
            }
        } break;

        case 'n': {
            // "null".
            if (cos_tokenizer_match(tokenizer, 'u') &&
                cos_tokenizer_match(tokenizer, 'l') &&
                cos_tokenizer_match(tokenizer, 'l')) {
                token->type = CosToken_Type_Keyword_Null;
            }
            else {
                // Unexpected character.
            }
        } break;

        case 'R': {
            // "R" (indirect object reference).
            token->type = CosToken_Type_Keyword_R;
        } break;

        case 'o': {
            // "obj" (beginning of an object).
            if (cos_tokenizer_match(tokenizer, 'b') &&
                cos_tokenizer_match(tokenizer, 'j')) {
                token->type = CosToken_Type_Keyword_Obj;
            }
            else {
                // Unexpected character.
            }
        } break;

        case 'e': {
            // "endobj" (end of an object) or "endstream".
            if (cos_tokenizer_match(tokenizer, 'n') &&
                cos_tokenizer_match(tokenizer, 'd') &&
                cos_tokenizer_match(tokenizer, 'o') &&
                cos_tokenizer_match(tokenizer, 'b') &&
                cos_tokenizer_match(tokenizer, 'j')) {
                token->type = CosToken_Type_Keyword_EndObj;
            }
            else if (cos_tokenizer_match(tokenizer, 'n') &&
                     cos_tokenizer_match(tokenizer, 'd') &&
                     cos_tokenizer_match(tokenizer, 's') &&
                     cos_tokenizer_match(tokenizer, 't') &&
                     cos_tokenizer_match(tokenizer, 'r') &&
                     cos_tokenizer_match(tokenizer, 'e') &&
                     cos_tokenizer_match(tokenizer, 'a') &&
                     cos_tokenizer_match(tokenizer, 'm')) {
                token->type = CosToken_Type_Keyword_EndStream;
            }
            else {
                // Unexpected character.
            }
        } break;

        case 's': {
            // "stream" (beginning of a stream) or "startxref".
            if (cos_tokenizer_match(tokenizer, 't')) {
                if (cos_tokenizer_match(tokenizer, 'r') &&
                    cos_tokenizer_match(tokenizer, 'e') &&
                    cos_tokenizer_match(tokenizer, 'a') &&
                    cos_tokenizer_match(tokenizer, 'm')) {
                    token->type = CosToken_Type_Keyword_Stream;
                }
                else if (cos_tokenizer_match(tokenizer, 'a') &&
                         cos_tokenizer_match(tokenizer, 'r') &&
                         cos_tokenizer_match(tokenizer, 't') &&
                         cos_tokenizer_match(tokenizer, 'x') &&
                         cos_tokenizer_match(tokenizer, 'r') &&
                         cos_tokenizer_match(tokenizer, 'e') &&
                         cos_tokenizer_match(tokenizer, 'f')) {
                    token->type = CosToken_Type_Keyword_StartXRef;
                }
                else {
                    // Unexpected character.
                }
            }
            else {
                // Unexpected character.
            }
        } break;

        case 'x': {
            // "xref" (cross-reference table).
            if (cos_tokenizer_match(tokenizer, 'r') &&
                cos_tokenizer_match(tokenizer, 'e') &&
                cos_tokenizer_match(tokenizer, 'f')) {
                token->type = CosToken_Type_Keyword_XRef;
            }
            else {
                // Unexpected character.
            }
        } break;

        default:
            // Unexpected character.
            break;
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

static int
cos_tokenizer_get_next_char(CosTokenizer *tokenizer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    return cos_input_stream_reader_getc(tokenizer->input_stream_reader);
}

static int
cos_tokenizer_peek_next_char(CosTokenizer *tokenizer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);

    return cos_input_stream_reader_peek(tokenizer->input_stream_reader);
}

static bool
cos_tokenizer_accept(CosTokenizer *tokenizer,
                     char character)
{
    if (!tokenizer) {
        return false;
    }

    return cos_data_buffer_push_back(tokenizer->text_buffer,
                                     (CosByte)character,
                                     NULL);
}

static CosToken
cos_tokenizer_make_token(CosTokenizer *tokenizer,
                         CosToken_Type type,
                         CosTokenValue *value)
{

    const CosToken token = {
        .type = type,
        .text = {0},
        //        .value = value,
    };
    return token;
}

static bool
cos_tokenizer_match(CosTokenizer *tokenizer,
                    char character)
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
cos_tokenizer_read_name_(CosTokenizer *tokenizer,
                         CosDataBuffer *buffer)
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

        cos_data_buffer_push_back(buffer,
                                  (unsigned char)character,
                                  NULL);
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
cos_tokenizer_read_literal_string_(CosTokenizer *tokenizer,
                                   CosDataBuffer *buffer)
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
                if (cos_tokenizer_match(tokenizer,
                                        CosCharacterSet_LineFeed)) {
                    // Treating the CRLF as LF.
                }
                c = CosCharacterSet_LineFeed;
            } break;

            default:
                break;
        }

        cos_data_buffer_push_back(buffer,
                                  (unsigned char)c,
                                  NULL);

    continue_label:;
    }

    // Error: unterminated literal string.
    return false;
}

static bool
cos_tokenizer_handle_literal_string_escape_sequence_(CosTokenizer *tokenizer,
                                                     unsigned char *result)
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
            if (cos_tokenizer_match(tokenizer,
                                    CosCharacterSet_LineFeed)) {
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
cos_tokenizer_read_hex_string_(CosTokenizer *tokenizer,
                               CosDataBuffer *buffer,
                               CosError **error)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    COS_PARAMETER_ASSERT(buffer != NULL);

    int character = EOF;
    while ((character = cos_tokenizer_get_next_char(tokenizer)) != EOF) {
        if (character == CosCharacterSet_GreaterThanSign) {
            // This is the end of the hex string.
            return true;
        }
        else if (cos_is_whitespace(character)) {
            // Whites-space characters shall be ignored.
            continue;
        }
        else if (!cos_is_hex_digit(character)) {
            // Skip all non-hex characters.
            continue;
        }

        cos_data_buffer_push_back(buffer,
                                  (unsigned char)character,
                                  NULL);
    }

    // Error: unterminated hex string.
    if (error) {
        *error = cos_error_alloc(COS_ERROR_SYNTAX,
                                 "Unterminated hex string");
    }
    return false;
}

static char *
cos_tokenizer_read_number(CosTokenizer *tokenizer)
{
    return NULL;
}

static char *
cos_tokenizer_read_real(CosTokenizer *tokenizer)
{
    return NULL;
}

static CosString *
cos_tokenizer_read_comment(CosTokenizer *tokenizer)
{
    COS_PARAMETER_ASSERT(tokenizer != NULL);
    if (!tokenizer) {
        return NULL;
    }

    CosString * const string = cos_string_alloc();

    const int c = cos_tokenizer_get_next_char(tokenizer);
    if (c == EOF) {
        return NULL;
    }
    else if (c == CosCharacterSet_PercentSign) {
        //        comment = cos_input_stream_read_line(input_stream);
    }

    return string;
}

COS_ASSUME_NONNULL_END
