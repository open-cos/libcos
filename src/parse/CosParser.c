//
// Created by david on 29/06/23.
//

#include "CosParser.h"

#include "common/Assert.h"
#include "libcos/CosDocument.h"
#include "libcos/common/CosTypes.h"
#include "parse/CosTokenizer.h"
#include <libcos/CosObject.h>
#include <libcos/common/CosError.h>
#include <libcos/io/CosInputStream.h>

#include "CosToken.h"

#include <stdlib.h>

struct CosParser {
    CosTokenizer *tokenizer;
};

static CosObject *
cos_parser_next_object(CosParser *parser,
                       CosError **error);

static CosObject *
cos_parser_handle_literal_string(CosParser *parser,
                                 CosError **error);

static CosObject *
cos_parser_handle_array(CosParser *parser,
                        const CosToken *start_token,
                        CosError **error);

CosParser *
cos_parser_alloc(CosInputStream *input_stream)
{
    if (!input_stream) {
        return NULL;
    }

    CosParser * const parser = malloc(sizeof(CosParser));
    if (!parser) {
        goto fail;
    }

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

CosObject *
cos_parser_next_object(CosParser *parser,
                       CosError **error)
{
    if (!parser) {
        cos_error_propagate(cos_error_alloc(COS_ERROR_INVALID_ARGUMENT,
                                            "parser is NULL"),
                            error);
        return NULL;
    }

    CosObject *object = NULL;

    const CosToken *token = NULL;
    while ((token = cos_tokenizer_next_token(parser->tokenizer)) != NULL) {

        switch (cos_token_get_type(token)) {
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
            case CosToken_Type_Comment:
                break;

            case CosToken_Type_EOF:
                break;
        }
    }

    return object;
}

CosObject *
cos_parser_handle_literal_string(CosParser *parser,
                                 CosError **error)
{
    COS_ASSERT(parser != NULL, "Expected a non-NULL parser");

    CosToken * const token = cos_tokenizer_next_token(parser->tokenizer);
    if (token) {
//        const char * const string = cos_token_get_string_value(token);
//        if (string) {
//
//        }
    }

    return NULL;
}

static CosObject *
cos_parser_handle_array(CosParser *parser,
                        const CosToken *start_token,
                        CosError **error)
{
    COS_ASSERT(parser != NULL, "Expected a non-NULL parser");

    CosError *local_error = NULL;

    CosObject * const array_object = cos_array_object_alloc();

    CosToken *token = NULL;
    while ((token = cos_tokenizer_peek_token(parser->tokenizer)) != NULL) {
        // Check for array end.
        if (cos_token_get_type(token) == CosToken_Type_ArrayEnd) {
            // Consume the peeked token.
            cos_token_free(cos_tokenizer_next_token(parser->tokenizer));
            break;
        }

        // Parse the next object.
        CosObject * const object = cos_parser_next_object(parser,
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

    return array_object;
}
