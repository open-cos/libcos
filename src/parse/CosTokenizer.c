//
// Created by david on 03/06/23.
//

#include "CosTokenizer.h"

#include "common/Assert.h"
#include "common/CharacterSet.h"
#include "common/CosString.h"
#include "libcos/io/CosInputStream.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

struct CosSourceLocation {
    unsigned int line;
    unsigned int column;
};

struct CosTokenizer {
    CosInputStream *input_stream;

    int peeked;
    bool has_peeked;
};

struct CosToken {
    CosToken_Type type;
    CosString *value;
};

static int
cos_tokenizer_get_next_char(CosTokenizer *tokenizer);

static int
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
        return NULL;
    }

    tokenizer->input_stream = input_stream;
    tokenizer->peeked = EOF;
    tokenizer->has_peeked = false;

    return tokenizer;
}

void
cos_tokenizer_free(CosTokenizer *tokenizer)
{
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

        default:
            // Unexpected character.
            break;
    }

    return token;
}

static int
cos_tokenizer_get_next_char(CosTokenizer *tokenizer)
{
    if (!tokenizer) {
        return EOF;
    }

    if (tokenizer->has_peeked) {
        tokenizer->has_peeked = false;
        return tokenizer->peeked;
    }
    else {
        return cos_input_stream_getc(tokenizer->input_stream);
    }
}

static int
cos_tokenizer_peek_next_char(CosTokenizer *tokenizer)
{
    if (!tokenizer) {
        return EOF;
    }

    if (!tokenizer->has_peeked) {
        tokenizer->peeked = cos_input_stream_getc(tokenizer->input_stream);
        tokenizer->has_peeked = true;
    }

    return tokenizer->peeked;
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

        cos_string_append_char(string, (char)c);
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
