//
// Created by david on 29/06/23.
//

#include "CosParser.h"

#include "common/Assert.h"
#include "libcos/CosArrayObj.h"
#include "libcos/CosDocument.h"
#include "libcos/CosStringObj.h"
#include "libcos/common/CosTypes.h"
#include "parse/CosTokenizer.h"

#include <libcos/CosObj.h>
#include <libcos/common/CosError.h>
#include <libcos/io/CosInputStream.h>

#include <stdlib.h>

#include "CosToken.h"

struct CosParser {
    CosDocument *document;
    CosTokenizer *tokenizer;
};

static CosObj *
cos_parser_next_object(CosParser *parser,
                       CosError **error);

static CosObj *
cos_parser_handle_literal_string(CosParser *parser,
                                 CosError **error);

static CosObj *
cos_parser_handle_array(CosParser *parser,
                        const CosToken *start_token,
                        CosError **error);

CosObj *
cos_string_object_alloc(CosDocument *
                            document,
                        char * const
                            string,
                        size_t
                            length);

static bool
cos_tokenizer_is_at_end(CosTokenizer *
                            tokenizer);

CosParser *
cos_parser_alloc(CosDocument *document,
                 CosInputStream *input_stream)
{
    if (!input_stream) {
        return NULL;
    }

    CosParser * const parser = malloc(sizeof(CosParser));
    if (!parser) {
        goto fail;
    }

    parser->document = document;

    parser->tokenizer = cos_tokenizer_alloc(input_stream);
    if (!parser->tokenizer) {
        goto fail;
    }

    return parser;

fail:
    if (parser) {
        cos_parser_free(parser);
    }
    return NULL;
}

void
cos_parser_free(CosParser *parser)
{
    if (!parser) {
        return;
    }

    cos_tokenizer_free(parser->tokenizer);
    parser->tokenizer = NULL;

    free(parser);
}

CosObj *
cos_parser_next_object(CosParser *parser,
                       CosError **error)
{
    if (!parser) {
        cos_error_propagate(cos_error_alloc(COS_ERROR_INVALID_ARGUMENT,
                                            "parser is NULL"),
                            error);
        return NULL;
    }

    CosObj *object = NULL;

    const CosToken *token = NULL;
    while ((token = cos_tokenizer_next_token(parser->tokenizer)) != NULL) {

        switch (token->type) {
            case CosToken_Type_Unknown:
                break;
            case CosToken_Type_Boolean:
                break;
            case CosToken_Type_Literal_String: {
                object = cos_parser_handle_literal_string(parser,
                                                          error);
            } break;
            case CosToken_Type_Hex_String:
                break;
            case CosToken_Type_Name:
                break;
            case CosToken_Type_Integer:
                break;
            case CosToken_Type_Real:
                break;
            case CosToken_Type_Stream:
                break;
            case CosToken_Type_Null:
                break;
            case CosToken_Type_EndOfLine:
                break;
            case CosToken_Type_ArrayStart: {
                object = cos_parser_handle_array(parser,
                                                 token,
                                                 error);
            } break;
            case CosToken_Type_ArrayEnd:
                break;
            case CosToken_Type_DictionaryStart:
                break;
            case CosToken_Type_DictionaryEnd:
                break;

            case CosToken_Type_EOF:
                break;
        }
    }

    return object;
}

CosObj *
cos_parser_handle_literal_string(CosParser *parser,
                                 CosError **error)
{
    COS_ASSERT(parser != NULL, "Expected a non-NULL parser");

    CosToken * const token = cos_tokenizer_next_token(parser->tokenizer);
    if (token) {
        size_t string_length = 0;
        char * const string = cos_token_copy_string_value(token,
                                                          &string_length);
        if (string) {
            CosObj * const string_object = (CosObj *)cos_string_obj_alloc(string,
                                                                          string_length);
            free(string);
            return string_object;
        }
    }

    return NULL;
}

CosObj *
cos_string_object_alloc(CosDocument *document,
                        char * const string,
                        size_t length)
{
    return NULL;
}

static CosObj *
cos_parser_handle_array(CosParser *parser,
                        const CosToken *start_token,
                        CosError **error)
{
    COS_ASSERT(parser != NULL, "Expected a non-NULL parser");

    CosError *local_error = NULL;

    CosArrayObj * const array_object = cos_array_obj_alloc();

    while (!cos_tokenizer_is_at_end_(parser->tokenizer)) {
        {
            CosToken *peeked_token = cos_tokenizer_peek_token(parser->tokenizer);
            if (!peeked_token) {
                goto loop_fail;
            }
            if (peeked_token->type == CosToken_Type_ArrayEnd) {
                // Consume the peeked token.
                cos_tokenizer_next_token(parser->tokenizer);
                break;
            }
        }
    }

    CosToken *token = NULL;
    while ((token = cos_tokenizer_peek_token(parser->tokenizer)) != NULL) {
        // Check for array end.
        if (token->type == CosToken_Type_ArrayEnd) {
            // Consume the peeked token.
            cos_tokenizer_next_token(parser->tokenizer);
            break;
        }

        // Parse the next object.
        CosObj * const object = cos_parser_next_object(parser,
                                                       &local_error);
        if (!object) {
            goto loop_fail;
        }

        // Add the object to the array.
        if (!cos_array_object_append(array_object,
                                     object,
                                     &local_error)) {
            goto loop_fail;
        }

        continue;

    loop_fail:
        cos_error_propagate(local_error,
                            error);
        //        cos_object_free(array_object);
        return NULL;
    }

    return (CosObj *)array_object;
}

static bool
cos_tokenizer_is_at_end(CosTokenizer *tokenizer)
{
    return 0;
}
