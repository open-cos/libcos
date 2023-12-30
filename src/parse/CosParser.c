//
// Created by david on 29/06/23.
//

#include "CosParser.h"

#include "common/Assert.h"
#include "libcos/CosDocument.h"
#include "libcos/objects/CosDictObj.h"
#include "parse/CosToken.h"
#include "parse/CosTokenizer.h"

#include <libcos/CosArrayObj.h>
#include <libcos/CosBoolObj.h>
#include <libcos/CosIntegerObj.h>
#include <libcos/CosNameObj.h>
#include <libcos/CosNullObj.h>
#include <libcos/CosObjID.h>
#include <libcos/CosRealObj.h>
#include <libcos/CosStringObj.h>
#include <libcos/common/CosDiagnosticHandler.h>
#include <libcos/common/CosError.h>
#include <libcos/io/CosInputStream.h>

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

struct CosParser {
    CosDocument *document;
    CosTokenizer *tokenizer;

    int integers[3];
    size_t integer_count;

    CosDiagnosticHandler *diagnostic_handler;
};

static bool
cos_parser_init_(CosParser *self,
                 CosDocument *document,
                 CosInputStream *input_stream);

static CosBaseObj * COS_Nullable
cos_parser_next_object_(CosParser *parser,
                        CosError * COS_Nullable error);

static CosBaseObj * COS_Nullable
cos_parser_handle_literal_string_(CosParser *parser,
                                  CosToken *token,
                                  CosError * COS_Nullable error);

static CosBaseObj * COS_Nullable
cos_parser_handle_hex_string_(CosParser *parser,
                              CosToken *token,
                              CosError * COS_Nullable error);

static CosBaseObj * COS_Nullable
cos_parser_handle_name_(CosParser *parser,
                        CosToken *token,
                        CosError * COS_Nullable error);

static CosBaseObj * COS_Nullable
cos_parser_handle_integer_(CosParser *parser,
                           const CosToken *token,
                           CosError * COS_Nullable error);

static CosBaseObj * COS_Nullable
cos_parser_handle_real_(CosParser *parser,
                        const CosToken *token,
                        CosError * COS_Nullable error);

static CosBaseObj * COS_Nullable
cos_parser_handle_array_(CosParser *parser,
                         CosError * COS_Nullable error);

static CosBaseObj * COS_Nullable
cos_parser_handle_dictionary_(CosParser *parser,
                              CosError * COS_Nullable error);

static CosBaseObj * COS_Nullable
cos_parser_handle_keyword_(CosParser *parser,
                           const CosToken *keyword_token,
                           CosError * COS_Nullable error);

static CosBaseObj * COS_Nullable
cos_parser_handle_indirect_obj_def_(CosParser *parser,
                                    CosError * COS_Nullable error);

static CosBaseObj * COS_Nullable
cos_parser_handle_indirect_obj_ref_(CosParser *parser,
                                    CosError * COS_Nullable error);

CosParser *
cos_parser_alloc(CosDocument *document,
                 CosInputStream *input_stream)
{
    COS_PARAMETER_ASSERT(document != NULL);
    COS_PARAMETER_ASSERT(input_stream != NULL);
    if (!input_stream) {
        return NULL;
    }

    CosParser * const parser = malloc(sizeof(CosParser));
    if (!parser) {
        goto failure;
    }

    if (!cos_parser_init_(parser,
                          document,
                          input_stream)) {
        goto failure;
    }

    return parser;

failure:
    if (parser) {
        free(parser);
    }
    return NULL;
}

static bool
cos_parser_init_(CosParser *self,
                 CosDocument *document,
                 CosInputStream *input_stream)
{
    COS_PARAMETER_ASSERT(self != NULL);
    COS_PARAMETER_ASSERT(document != NULL);
    COS_PARAMETER_ASSERT(input_stream != NULL);
    if (!self || !input_stream) {
        return false;
    }

    CosTokenizer * const tokenizer = cos_tokenizer_alloc(input_stream);
    if (!tokenizer) {
        goto failure;
    }

    self->document = document;
    self->tokenizer = tokenizer;

    return true;

failure:
    if (tokenizer) {
        cos_tokenizer_free(tokenizer);
    }
    return false;
}

void
cos_parser_free(CosParser *parser)
{
    if (!parser) {
        return;
    }

    cos_tokenizer_free(parser->tokenizer);

    free(parser);
}

CosBaseObj *
cos_parser_next_object(CosParser *parser,
                       CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    return cos_parser_next_object_(parser, error);
}

#pragma mark - Implementation

static CosBaseObj *
cos_parser_next_object_(CosParser *parser,
                        CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    CosToken * const token = cos_tokenizer_next_token(parser->tokenizer);
    if (!token) {
        return NULL;
    }

    CosBaseObj *object = NULL;

    switch (token->type) {
        case CosToken_Type_Unknown:
            break;

        case CosToken_Type_Literal_String: {
            object = cos_parser_handle_literal_string_(parser, token, error);
        } break;
        case CosToken_Type_Hex_String: {
            object = cos_parser_handle_hex_string_(parser, token, error);
        } break;

        case CosToken_Type_Name: {
            object = cos_parser_handle_name_(parser, token, error);
        } break;

        case CosToken_Type_Integer: {
            object = cos_parser_handle_integer_(parser, token, error);
        } break;
        case CosToken_Type_Real: {
            object = cos_parser_handle_real_(parser, token, error);
        } break;

        case CosToken_Type_ArrayStart: {
            object = cos_parser_handle_array_(parser, error);
        } break;
        case CosToken_Type_ArrayEnd:
            break;

        case CosToken_Type_DictionaryStart: {
            object = cos_parser_handle_dictionary_(parser, error);
        } break;
        case CosToken_Type_DictionaryEnd:
            break;

        case CosToken_Type_Keyword: {
            object = cos_parser_handle_keyword_(parser, token, error);
        } break;

        case CosToken_Type_EOF:
            break;
    }

    return object;
}

static CosBaseObj *
cos_parser_handle_literal_string_(CosParser *parser,
                                  CosToken *token,
                                  CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);

    CosData *string_data = NULL;
    if (cos_token_value_take_data(&(token->value),
                                  &string_data)) {
        CosBaseObj * const string_object = (CosBaseObj *)cos_string_obj_alloc(string_data,
                                                                              parser->document);
        return string_object;
    }
    else {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid literal string token"),
                            error);
    }

    return NULL;
}

static CosBaseObj *
cos_parser_handle_hex_string_(CosParser *parser,
                              CosToken *token,
                              CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);

    CosData *string_data = NULL;
    if (cos_token_value_take_data(&(token->value),
                                  &string_data)) {
        CosBaseObj * const string_object = (CosBaseObj *)cos_string_obj_alloc(string_data,
                                                                              parser->document);
        return string_object;
    }
    else {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid hexadecimal string token"),
                            error);
    }

    return NULL;
}

static CosBaseObj *
cos_parser_handle_name_(CosParser *parser,
                        CosToken *token,
                        CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);

    CosString *name = NULL;
    if (cos_token_value_take_string(&(token->value), &name)) {
        return (CosBaseObj *)cos_name_obj_create(name);
    }
    else {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid name token"),
                            error);
        return NULL;
    }
}

static CosBaseObj *
cos_parser_handle_integer_(CosParser *parser,
                           const CosToken *token,
                           CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    int integer_value = 0;
    if (cos_token_value_get_integer_number(&(token->value),
                                           &integer_value)) {
        parser->integers[parser->integer_count++] = integer_value;
    }
    else {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid integer token"),
                            error);
        return NULL;
    }

    if (parser->integer_count == 3) {
        CosBaseObj * const int_obj = (CosBaseObj *)cos_integer_obj_alloc(parser->integers[0]);
        memmove(parser->integers,
                parser->integers + 1,
                sizeof(parser->integers[0]) * 2);
        parser->integer_count--;
        return int_obj;
    }

    return cos_parser_next_object_(parser, error);
}

static CosBaseObj *
cos_parser_handle_real_(CosParser *parser,
                        const CosToken *token,
                        CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(token != NULL);
    if (!parser) {
        return NULL;
    }

    double real_value = 0.0;
    if (cos_token_value_get_real_number(&(token->value),
                                        &real_value)) {
        return (CosBaseObj *)cos_real_obj_alloc(real_value,
                                                parser->document);
    }
    else {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid real token"),
                            error);
        return NULL;
    }
}

static CosBaseObj *
cos_parser_handle_array_(CosParser *parser,
                         CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    CosArrayObj * const array_object = cos_array_obj_create();
    if (!array_object) {
        goto failure;
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
        CosBaseObj * const object = cos_parser_next_object_(parser,
                                                            error);
        if (!object) {
            goto failure;
        }

        // Add the object to the array.
        if (!cos_array_object_append(array_object,
                                     object,
                                     error)) {
            goto failure;
        }
    }

    return (CosBaseObj *)array_object;

failure:
    return NULL;
}

static CosBaseObj *
cos_parser_handle_dictionary_(CosParser *parser,
                              CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    CosDictObj * const dict_obj = cos_dict_obj_create(NULL);
    if (!dict_obj) {
        goto failure;
    }

    CosToken *token = NULL;
    while ((token = cos_tokenizer_peek_token(parser->tokenizer)) != NULL) {
        // Check for dictionary end.
        if (token->type == CosToken_Type_DictionaryEnd) {
            // Consume the peeked token.
            cos_tokenizer_next_token(parser->tokenizer);
            break;
        }

        // Parse the next object.
        CosBaseObj * const key = cos_parser_next_object_(parser,
                                                         error);
        if (!key) {
            goto failure;
        }

        // Parse the next object.
        CosBaseObj * const value = cos_parser_next_object_(parser,
                                                           error);
        if (!value) {
            goto failure;
        }

        // Add the object to the dictionary.
        if (!cos_dict_obj_set(dict_obj,
                              key,
                              value,
                              error)) {
            goto failure;
        }
    }

    return (CosBaseObj *)dict_obj;

failure:
    return NULL;
}

static CosBaseObj * COS_Nullable
cos_parser_handle_keyword_(CosParser *parser,
                           const CosToken *keyword_token,
                           CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(keyword_token != NULL && keyword_token->type == CosToken_Type_Keyword);
    if (!parser || !keyword_token) {
        return NULL;
    }

    CosKeywordType keyword_type;
    if (!cos_token_value_get_keyword(&(keyword_token->value),
                                     &keyword_type)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid keyword token"),
                            error);
        return NULL;
    }

    switch (keyword_type) {
        case CosKeywordType_Unknown:
            break;

        case CosKeywordType_True: {
            return (CosBaseObj *)cos_bool_obj_alloc(true);
        }
        case CosKeywordType_False: {
            return (CosBaseObj *)cos_bool_obj_alloc(false);
        }

        case CosKeywordType_Null: {
            return (CosBaseObj *)cos_null_obj_get();
        }

        case CosKeywordType_R: {
            return cos_parser_handle_indirect_obj_ref_(parser, error);
        }
        case CosKeywordType_Obj: {
            return cos_parser_handle_indirect_obj_def_(parser, error);
        }

        case CosKeywordType_EndObj:
            break;
        case CosKeywordType_Stream:
            break;
        case CosKeywordType_EndStream:
            break;
        case CosKeywordType_XRef:
            break;
        case CosKeywordType_Trailer:
            break;
        case CosKeywordType_StartXRef:
            break;
    }

    return NULL;
}

static CosBaseObj * COS_Nullable
cos_parser_handle_indirect_obj_def_(CosParser *parser,
                                    CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    // TODO: We should have two integers on the stack.

    // The value of the indirect object follows the "obj" keyword.
    CosBaseObj * const obj = cos_parser_next_object_(parser, error);
    if (!obj) {
        cos_diagnose(parser->diagnostic_handler,
                     CosDiagnosticLevel_Error,
                     "Invalid indirect object definition");
        return NULL;
    }

    if (!cos_tokenizer_get_next_keyword_if(parser->tokenizer,
                                           CosKeywordType_EndObj)) {
        cos_diagnose(parser->diagnostic_handler,
                     CosDiagnosticLevel_Warning,
                     "Unterminated indirect object definition");
    }

    return obj;
}

static CosBaseObj * COS_Nullable
cos_parser_handle_indirect_obj_ref_(CosParser *parser,
                                    CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    // TODO: We should have two integers on the stack.
    if (parser->integer_count != 2) {
        cos_diagnose(parser->diagnostic_handler,
                     CosDiagnosticLevel_Error,
                     "Invalid indirect object reference");
        return NULL;
    }

    int object_number = parser->integers[0];
    int generation_number = parser->integers[1];

    parser->integer_count = 0;

    const CosObjID obj_id = cos_obj_id_make(object_number,
                                            generation_number);

    return cos_document_get_object(parser->document, obj_id, error);
}

COS_ASSUME_NONNULL_END
