//
// Created by david on 03/06/23.
//

#include "CosTokenizer.h"

#include "common/Assert.h"
#include "common/CharacterSet.h"
#include "common/CosString.h"
#include "common/CosVector.h"
#include "libcos/io/CosInputStream.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "CosToken.h"

struct CosSourceLocation {
    unsigned int line;
    unsigned int column;
};

struct CosTokenizer {
    CosInputStream *input_stream;

    int peeked_char;
    bool has_peeked_char;

    CosToken *peeked_token;

    CosToken current_token;
    CosToken peeked_tokens;

    CosVector *text_buffer;
};

void
cos_token_free(CosToken *token)
{
    //    if (token->value) {
    //        cos_string_free(token->value);
    //    }

    free(token);
}

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

static CosString *
cos_tokenizer_read_name(CosTokenizer *tokenizer);

static CosString *
cos_tokenizer_read_literal_string(CosTokenizer *tokenizer);

static int
cos_tokenizer_handle_literal_string_escape_sequence(CosTokenizer *tokenizer);

static CosString *
cos_tokenizer_read_hex_string(CosTokenizer *tokenizer);

static char *
cos_tokenizer_read_number(CosTokenizer *tokenizer);

static char *
cos_tokenizer_read_real(CosTokenizer *tokenizer);

static CosString *
cos_tokenizer_read_comment(CosTokenizer *tokenizer);

CosTokenizer *
cos_tokenizer_alloc(CosInputStream *input_stream)
{
    CosTokenizer * const tokenizer = malloc(sizeof(CosTokenizer));
    if (!tokenizer) {
        goto fail;
    }

    tokenizer->input_stream = input_stream;
    tokenizer->peeked_char = EOF;
    tokenizer->has_peeked_char = false;
    tokenizer->peeked_token = NULL;

    tokenizer->text_buffer = cos_vector_alloc(sizeof(char));
    if (!tokenizer->text_buffer) {
        goto fail;
    }

    return tokenizer;

fail:
    if (tokenizer) {
        cos_tokenizer_free(tokenizer);
    }
    return NULL;
}

void
cos_tokenizer_free(CosTokenizer *tokenizer)
{
    if (!tokenizer) {
        return;
    }

    if (tokenizer->peeked_token) {
        cos_token_free(tokenizer->peeked_token);
    }

    free(tokenizer);
}

CosInputStream *
cos_tokenizer_get_input_stream(const CosTokenizer *tokenizer)
{
    return tokenizer->input_stream;
}

CosToken *
cos_tokenizer_next_token(CosTokenizer *tokenizer)
{
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
    token->value = NULL;

    int c = cos_tokenizer_get_next_char(tokenizer);
    switch (c) {
        case CosCharacterSet_LeftSquareBracket: {
            token->type = CosToken_Type_ArrayStart;
        } break;
        case CosCharacterSet_RightSquareBracket: {
            token->type = CosToken_Type_ArrayEnd;
        } break;

        case CosCharacterSet_Solidus: {
            token->type = CosToken_Type_Name;
            token->value = cos_tokenizer_read_name(tokenizer);
        } break;

        case CosCharacterSet_LeftParenthesis: {
            token->type = CosToken_Type_Literal_String;
            token->value = cos_tokenizer_read_literal_string(tokenizer);
        } break;

        case CosCharacterSet_LessThanSign: {
            if (cos_tokenizer_match(tokenizer, CosCharacterSet_LessThanSign)) {
                token->type = CosToken_Type_DictionaryStart;
            }
            else {
                token->type = CosToken_Type_Hex_String;
                token->value = cos_tokenizer_read_hex_string(tokenizer);
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
            token->value = cos_tokenizer_read_comment(tokenizer);
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
    if (!tokenizer) {
        return EOF;
    }

    int c = 0;

    if (tokenizer->has_peeked_char) {
        tokenizer->has_peeked_char = false;
        c = tokenizer->peeked_char;
    }
    else {
        c = cos_input_stream_getc(tokenizer->input_stream);
    }

    cos_vector_add_element(tokenizer->text_buffer, &c);

    return c;
}

static int
cos_tokenizer_peek_next_char(CosTokenizer *tokenizer)
{
    if (!tokenizer) {
        return EOF;
    }

    if (!tokenizer->has_peeked_char) {
        tokenizer->peeked_char = cos_input_stream_getc(tokenizer->input_stream);
        tokenizer->has_peeked_char = true;
    }

    return tokenizer->peeked_char;
}

static bool
cos_tokenizer_accept(CosTokenizer *tokenizer,
                     char character)
{
    if (!tokenizer) {
        return false;
    }

    return cos_vector_add_element(tokenizer->text_buffer,
                                  &character);
}

static CosToken
cos_tokenizer_make_token(CosTokenizer *tokenizer,
                         CosToken_Type type,
                         CosTokenValue *value)
{


    const CosToken token = {
        .type = type,
        .value = value,
    };
    return token;
}

static bool
cos_tokenizer_match(CosTokenizer *tokenizer,
                    char character)
{
    if (!tokenizer) {
        return false;
    }

    int c = cos_tokenizer_peek_next_char(tokenizer);
    if (c == character) {
        cos_tokenizer_get_next_char(tokenizer);

        return true;
    }
    else {
        return false;
    }
}

static CosString *
cos_tokenizer_read_name(CosTokenizer *tokenizer)
{
    if (!tokenizer) {
        return NULL;
    }

    CosString * const string = cos_string_alloc();

    int character;
    while ((character = cos_tokenizer_get_next_char(tokenizer)) != EOF) {
        if (cos_is_whitespace(character) ||
            cos_is_delimiter(character)) {
            break;
        }

        cos_string_append_char(string,
                               character);
    }

    return string;
}

static CosString *
cos_tokenizer_read_literal_string(CosTokenizer *tokenizer)
{
    if (!tokenizer) {
        return NULL;
    }

    CosString * const string = cos_string_alloc();

    unsigned int num_parentheses = 0;

    int c = EOF;
    while ((c = cos_input_stream_getc(tokenizer->input_stream)) != EOF) {
        if (c == CosCharacterSet_RightParenthesis) {
            if (num_parentheses > 0) {
                num_parentheses--;
            }
            else {
                // This is the end of the literal string.
                break;
            }
        }

        /*
         * An end-of-line marker appearing within a literal string without a preceding REVERSE SOLIDUS
         * shall be treated as a byte value of (0Ah), irrespective of whether the end-of-line marker was
         * a CARRIAGE RETURN (0Dh), a LINE FEED (0Ah), or both.
         */

        switch (c) {
            case CosCharacterSet_LeftParenthesis: {
                COS_ASSERT(num_parentheses < UINT_MAX,
                           "Maximum number of parentheses reached.");

                num_parentheses++;
            } break;

            case CosCharacterSet_ReverseSolidus: {
                cos_tokenizer_handle_literal_string_escape_sequence(tokenizer);
            } break;


            case CosCharacterSet_LineFeed: {

            } break;
            case CosCharacterSet_CarriageReturn: {
                if (cos_tokenizer_match(tokenizer,
                                        CosCharacterSet_LineFeed)) {
                    // Treating the CRLF as LF.
                }

                c = CosCharacterSet_LineFeed;
            } break;

            default:
                break;
        }

        cos_tokenizer_accept(tokenizer,
                             (char)c);
    }

    return string;
}

static int
cos_tokenizer_handle_literal_string_escape_sequence(CosTokenizer *tokenizer)
{
    if (!tokenizer) {
        return EOF;
    }

    const int c = cos_tokenizer_get_next_char(tokenizer);
    int result = EOF;
    switch (c) {
        case 'n': {
            result = CosCharacterSet_LineFeed;
        } break;
        case 'r': {
            result = CosCharacterSet_CarriageReturn;
        } break;
        case 't': {
            result = CosCharacterSet_HorizontalTab;
        } break;
        case 'b': {
            result = CosCharacterSet_Backspace;
        } break;
        case 'f': {
            result = CosCharacterSet_FormFeed;
        } break;

        case CosCharacterSet_LeftParenthesis: {
            result = CosCharacterSet_LeftParenthesis;
        } break;
        case CosCharacterSet_RightParenthesis: {
            result = CosCharacterSet_RightParenthesis;
        } break;
        case CosCharacterSet_ReverseSolidus: {
            result = CosCharacterSet_ReverseSolidus;
        } break;

        case CosCharacterSet_DigitZero:
        case CosCharacterSet_DigitOne:
        case CosCharacterSet_DigitTwo:
        case CosCharacterSet_DigitThree:
        case CosCharacterSet_DigitFour:
        case CosCharacterSet_DigitFive:
        case CosCharacterSet_DigitSix:
        case CosCharacterSet_DigitSeven: {
            int peeked = cos_input_stream_peek(tokenizer->input_stream);

        } break;

        case CosCharacterSet_LineFeed: {
            // This is a line continuation.
        } break;

        case CosCharacterSet_CarriageReturn: {
            // This is a line continuation.
            if (cos_tokenizer_match(tokenizer,
                                    CosCharacterSet_LineFeed)) {
                // Treating the CRLF as LF.
            }
        } break;


        default: {
            result = c;
        } break;
    }

    return result;
}

static CosString *
cos_tokenizer_read_hex_string(CosTokenizer *tokenizer)
{
    if (!tokenizer) {
        return NULL;
    }

    int c = EOF;
    while ((c = cos_input_stream_getc(tokenizer->input_stream)) != EOF) {
        if (c == CosCharacterSet_GreaterThanSign) {
            break;
        }
    }

    return NULL;
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
    CosInputStream * const input_stream = tokenizer->input_stream;

    CosString * const string = cos_string_alloc();

    const int c = cos_input_stream_getc(input_stream);
    if (c == EOF) {
        return NULL;
    }
    else if (c == CosCharacterSet_PercentSign) {
        //        comment = cos_input_stream_read_line(input_stream);
    }

    return string;
}
