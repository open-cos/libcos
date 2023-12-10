//
// Created by david on 14/11/23.
//

#include "CosLexer.h"

#include "common/Assert.h"
#include "common/CharacterSet.h"
#include "common/CosDataBuffer.h"
#include "parse/tokenizer/CosTokenValue.h"

#include <stdio.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

typedef struct CosTokenEntry CosTokenEntry;

struct CosTokenEntry {
    CosToken token;
    CosTokenValue value;
};

struct CosLexer {
    CosDocument *document;
    CosInputStream *input_stream;

    CosLocation location;

    int peeked_char;
    bool has_peeked_char;

    CosDataBuffer data_buffer;

    CosTokenEntry *token_buffer;
    size_t token_buffer_size;

    struct {
        unsigned int ignore_comments : 1;
    } flags;
};

static int
cos_lexer_peek_char_(CosLexer *lexer);

static int
cos_lexer_get_char_(CosLexer *lexer);

static bool
cos_lexer_match_(CosLexer *lexer,
                 int c);

static bool
cos_lexer_read_comment_(CosLexer *lexer,
                        CosDataBuffer * COS_Nullable buffer,
                        CosError **error_out);

static CosTokenValue
cos_lexer_read_number_(CosLexer *lexer);

static CosTokenValue
cos_lexer_read_real_(CosLexer *lexer);

static bool
cos_lexer_get_word_(CosLexer *lexer,
                    CosError **error_out);

CosLexer *
cos_lexer_alloc(CosDocument *document,
                CosInputStream *input_stream)
{
    CosLexer * const lexer = malloc(sizeof(CosLexer));
    if (!lexer) {
        return NULL;
    }

    lexer->document = document;
    lexer->input_stream = input_stream;

    cos_data_buffer_init(&lexer->data_buffer);

    lexer->flags.ignore_comments = false;

    return lexer;
}

void
cos_lexer_free(CosLexer *lexer)
{
    if (!lexer) {
        return;
    }

    cos_data_buffer_destroy(&lexer->data_buffer);

    free(lexer);
}

const CosInputStream *
cos_lexer_get_input_stream(const CosLexer *lexer)
{
    COS_ASSERT(lexer != NULL, "lexer cannot be NULL");

    return lexer->input_stream;
}

bool
cos_lexer_next_token(CosLexer *lexer,
                     CosToken *token_out,
                     CosError **error_out)
{
    COS_ASSERT(lexer != NULL, "lexer cannot be NULL");
    COS_ASSERT(token_out != NULL, "token_out cannot be NULL");

    CosToken token = {0};
    token.location = lexer->location;

    const int c = cos_lexer_get_char_(lexer);
    switch (c) {
        case CosCharacterSet_LeftSquareBracket: {
            token.type = CosToken_Type_ArrayStart;
        } break;
        case CosCharacterSet_RightSquareBracket: {
            token.type = CosToken_Type_ArrayEnd;
        } break;

        case CosCharacterSet_Solidus: {
            token.type = CosToken_Type_Name;
            //            token->value = cos_lexer_read_name(lexer);
        } break;

        case CosCharacterSet_LeftParenthesis: {
            token.type = CosToken_Type_Literal_String;
            //            token->value = cos_lexer_read_literal_string(lexer);
        } break;

        case CosCharacterSet_LessThanSign: {
            if (cos_lexer_match_(lexer, CosCharacterSet_LessThanSign)) {
                token.type = CosToken_Type_DictionaryStart;
            }
            else {
                token.type = CosToken_Type_Hex_String;
                //                token->value = cos_lexer_read_hex_string(lexer);
            }
        } break;

        case CosCharacterSet_GreaterThanSign: {
            if (cos_lexer_match_(lexer, CosCharacterSet_GreaterThanSign)) {
                token.type = CosToken_Type_DictionaryEnd;
            }
            else {
                // Unexpected character.
            }
        } break;

        case CosCharacterSet_PercentSign: {
            token.type = CosToken_Type_Comment;

            if (lexer->flags.ignore_comments) {
                // Ignore the comment.
                cos_lexer_read_comment_(lexer,
                                        NULL,
                                        error_out);
            }
            else {
                cos_lexer_read_comment_(lexer,
                                        &lexer->data_buffer,
                                        error_out);
            }

            //            token->value = cos_lexer_read_comment(lexer);
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
            cos_lexer_read_number_(lexer);
        } break;

        case CosCharacterSet_FullStop: {
            // We know that this is a real number.
            token.type = CosToken_Type_Real;
            cos_lexer_read_real_(lexer);
        } break;

        case EOF: {
            token.type = CosToken_Type_EOF;
        } break;

        case 't': {
            // "true" or "trailer".
            if (cos_lexer_match_(lexer, 'r')) {
                if (cos_lexer_match_(lexer, 'u') &&
                    cos_lexer_match_(lexer, 'e')) {
                    token.type = CosToken_Type_Keyword_True;
                }
                else if (cos_lexer_match_(lexer, 'a') &&
                         cos_lexer_match_(lexer, 'i') &&
                         cos_lexer_match_(lexer, 'l') &&
                         cos_lexer_match_(lexer, 'e') &&
                         cos_lexer_match_(lexer, 'r')) {
                    token.type = CosToken_Type_Keyword_Trailer;
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
            if (cos_lexer_match_(lexer, 'a') &&
                cos_lexer_match_(lexer, 'l') &&
                cos_lexer_match_(lexer, 's') &&
                cos_lexer_match_(lexer, 'e')) {
                token.type = CosToken_Type_Keyword_False;
            }
            else {
                // Unexpected character.
            }
        } break;

        case 'n': {
            // "null".
            if (cos_lexer_match_(lexer, 'u') &&
                cos_lexer_match_(lexer, 'l') &&
                cos_lexer_match_(lexer, 'l')) {
                token.type = CosToken_Type_Keyword_Null;
            }
            else {
                // Unexpected character.
            }
        } break;

        case 'R': {
            // "R" (indirect object reference).
            token.type = CosToken_Type_Keyword_R;
        } break;

        case 'o': {
            // "obj" (beginning of an object).
            if (cos_lexer_match_(lexer, 'b') &&
                cos_lexer_match_(lexer, 'j')) {
                token.type = CosToken_Type_Keyword_Obj;
            }
            else {
                // Unexpected character.
            }
        } break;

        case 'e': {
            // "endobj" (end of an object) or "endstream".
            if (cos_lexer_match_(lexer, 'n') &&
                cos_lexer_match_(lexer, 'd') &&
                cos_lexer_match_(lexer, 'o') &&
                cos_lexer_match_(lexer, 'b') &&
                cos_lexer_match_(lexer, 'j')) {
                token.type = CosToken_Type_Keyword_EndObj;
            }
            else if (cos_lexer_match_(lexer, 'n') &&
                     cos_lexer_match_(lexer, 'd') &&
                     cos_lexer_match_(lexer, 's') &&
                     cos_lexer_match_(lexer, 't') &&
                     cos_lexer_match_(lexer, 'r') &&
                     cos_lexer_match_(lexer, 'e') &&
                     cos_lexer_match_(lexer, 'a') &&
                     cos_lexer_match_(lexer, 'm')) {
                token.type = CosToken_Type_Keyword_EndStream;
            }
            else {
                // Unexpected character.
            }
        } break;

        case 's': {
            // "stream" (beginning of a stream) or "startxref".
            if (cos_lexer_match_(lexer, 't')) {
                if (cos_lexer_match_(lexer, 'r') &&
                    cos_lexer_match_(lexer, 'e') &&
                    cos_lexer_match_(lexer, 'a') &&
                    cos_lexer_match_(lexer, 'm')) {
                    token.type = CosToken_Type_Keyword_Stream;
                }
                else if (cos_lexer_match_(lexer, 'a') &&
                         cos_lexer_match_(lexer, 'r') &&
                         cos_lexer_match_(lexer, 't') &&
                         cos_lexer_match_(lexer, 'x') &&
                         cos_lexer_match_(lexer, 'r') &&
                         cos_lexer_match_(lexer, 'e') &&
                         cos_lexer_match_(lexer, 'f')) {
                    token.type = CosToken_Type_Keyword_StartXRef;
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
            if (cos_lexer_match_(lexer, 'r') &&
                cos_lexer_match_(lexer, 'e') &&
                cos_lexer_match_(lexer, 'f')) {
                token.type = CosToken_Type_Keyword_XRef;
            }
            else {
                // Unexpected character.
            }
        } break;

        default:
            // Unexpected character.
            break;
    }

    if (token_out) {
        *token_out = token;
    }
    return true;
}

static int
cos_lexer_peek_char_(CosLexer *lexer)
{
    if (!lexer->has_peeked_char) {
        lexer->peeked_char = cos_input_stream_peek(lexer->input_stream);
        lexer->has_peeked_char = true;
    }

    return lexer->peeked_char;
}

static int
cos_lexer_get_char_(CosLexer *lexer)
{
    int c;
    if (lexer->has_peeked_char) {
        lexer->has_peeked_char = false;
        c = lexer->peeked_char;
    }
    else {
        c = cos_input_stream_getc(lexer->input_stream);
    }

    if (c != EOF) {
        lexer->location.offset++;
    }

    return c;
}

static bool
cos_lexer_match_(CosLexer *lexer,
                 int c)
{
    const int peeked_char = cos_lexer_peek_char_(lexer);
    if (peeked_char == c) {
        cos_lexer_get_char_(lexer);
        return true;
    }

    return false;
}

static CosTokenValue
cos_lexer_read_number_(CosLexer *lexer)
{
    return (CosTokenValue){0};
}

static CosTokenValue
cos_lexer_read_real_(CosLexer *lexer)
{
    return (CosTokenValue){0};
}

static bool
cos_lexer_read_comment_(CosLexer *lexer,
                        CosDataBuffer * COS_Nullable buffer,
                        CosError **error_out)
{
    return false;
}



static bool
cos_lexer_get_word_(CosLexer *lexer,
                    CosError **error_out)
{
    COS_ASSERT(lexer != NULL, "lexer cannot be NULL");

    int peeked_char;
    while ((peeked_char = cos_lexer_peek_char_(lexer)) != EOF) {
        if (cos_is_delimiter(peeked_char) ||
            cos_is_whitespace(peeked_char)) {
            break;
        }

        int c = cos_lexer_get_char_(lexer);

        if (!cos_data_buffer_push_back(&lexer->data_buffer,
                                       (CosByte)c,
                                       error_out)) {
            break;
        }
    }

    return true;
}

COS_ASSUME_NONNULL_END
