/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "CosObjParser.h"

#include "common/Assert.h"
#include "common/CosMacros.h"
#include "parse/CosToken.h"
#include "parse/CosTokenizer.h"

#include <libcos/CosArrayObj.h>
#include <libcos/CosBoolObj.h>
#include <libcos/CosDoc.h>
#include <libcos/CosIntegerObj.h>
#include <libcos/CosNameObj.h>
#include <libcos/CosNullObj.h>
#include <libcos/CosObjID.h>
#include <libcos/CosRealObj.h>
#include <libcos/CosStringObj.h>
#include <libcos/common/CosDiagnosticHandler.h>
#include <libcos/common/CosError.h>
#include <libcos/io/CosInputStream.h>
#include <libcos/objects/CosDictObj.h>

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

struct CosObjParser {
    CosDoc *doc;
    CosTokenizer *tokenizer;

    unsigned int integers[3];
    size_t integer_count;

    CosBaseObj * COS_Nullable objects[3];
    size_t object_count;

    CosDiagnosticHandler *diagnostic_handler;
};

static bool
cos_obj_parser_init_(CosObjParser *self,
                     CosDoc *document,
                     CosInputStream *input_stream);

static CosBaseObj * COS_Nullable
cos_obj_parser_next_object_(CosObjParser *parser,
                            CosError * COS_Nullable error);

static CosBaseObj * COS_Nullable
cos_obj_parser_handle_literal_string_(CosObjParser *parser,
                                      CosToken *token,
                                      CosError * COS_Nullable error);

static CosBaseObj * COS_Nullable
cos_obj_parser_handle_hex_string_(CosObjParser *parser,
                                  CosToken *token,
                                  CosError * COS_Nullable error);

static CosBaseObj * COS_Nullable
cos_obj_parser_handle_name_(CosObjParser *parser,
                            CosToken *token,
                            CosError * COS_Nullable error);

static CosBaseObj * COS_Nullable
cos_obj_parser_handle_integer_(CosObjParser *parser,
                               const CosToken *token,
                               CosError * COS_Nullable error);

static CosBaseObj * COS_Nullable
cos_obj_parser_handle_real_(CosObjParser *parser,
                            const CosToken *token,
                            CosError * COS_Nullable error);

static CosBaseObj * COS_Nullable
cos_obj_parser_handle_array_(CosObjParser *parser,
                             CosError * COS_Nullable error);

static CosBaseObj * COS_Nullable
cos_obj_parser_handle_dictionary_(CosObjParser *parser,
                                  CosError * COS_Nullable error);

static CosBaseObj * COS_Nullable
cos_obj_parser_handle_keyword_(CosObjParser *parser,
                               const CosToken *token,
                               CosError * COS_Nullable error);

static CosBaseObj * COS_Nullable
cos_obj_parser_handle_indirect_obj_def_(CosObjParser *parser,
                                        CosError * COS_Nullable error);

static CosBaseObj * COS_Nullable
cos_obj_parser_handle_indirect_obj_ref_(CosObjParser *parser,
                                        CosError * COS_Nullable error);

static void
cos_obj_parser_push_int_(CosObjParser *parser,
                         unsigned int integer);

static unsigned int
cos_obj_parser_peek_int_(const CosObjParser *parser);

static unsigned int
cos_obj_parser_pop_int_(CosObjParser *parser);

static void
cos_obj_parser_push_object_(CosObjParser *parser,
                            CosBaseObj *object);

static CosBaseObj * COS_Nullable
cos_obj_parser_pop_object_(CosObjParser *parser);

CosObjParser *
cos_obj_parser_alloc(CosDoc *document,
                     CosInputStream *input_stream)
{
    COS_PARAMETER_ASSERT(document != NULL);
    COS_PARAMETER_ASSERT(input_stream != NULL);
    if (!input_stream) {
        return NULL;
    }

    CosObjParser * const parser = malloc(sizeof(CosObjParser));
    if (!parser) {
        goto failure;
    }

    if (!cos_obj_parser_init_(parser,
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
cos_obj_parser_init_(CosObjParser *self,
                     CosDoc *document,
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

    self->doc = document;
    self->tokenizer = tokenizer;

    self->integer_count = 0;
    self->object_count = 0;

    return true;

failure:
    if (tokenizer) {
        cos_tokenizer_free(tokenizer);
    }
    return false;
}

void
cos_obj_parser_free(CosObjParser *parser)
{
    if (!parser) {
        return;
    }

    cos_tokenizer_free(parser->tokenizer);

    for (size_t i = 0; i < parser->object_count; i++) {
        CosBaseObj * const object = parser->objects[i];
        cos_obj_release((CosObj *)object);
    }

    free(parser);
}

CosBaseObj *
cos_obj_parser_next_object(CosObjParser *parser,
                           CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    return cos_obj_parser_next_object_(parser, error);
}

#pragma mark - Implementation

static CosBaseObj *
cos_obj_parser_next_object_(CosObjParser *parser,
                            CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    CosToken token = cos_tokenizer_next_token(parser->tokenizer);

    CosBaseObj *object = NULL;

    switch (token.type) {
        case CosToken_Type_Unknown:
            break;

        case CosToken_Type_Literal_String: {
            object = cos_obj_parser_handle_literal_string_(parser, &token, error);
        } break;
        case CosToken_Type_Hex_String: {
            object = cos_obj_parser_handle_hex_string_(parser, &token, error);
        } break;

        case CosToken_Type_Name: {
            object = cos_obj_parser_handle_name_(parser, &token, error);
        } break;

        case CosToken_Type_Integer: {
            object = cos_obj_parser_handle_integer_(parser, &token, error);
        } break;
        case CosToken_Type_Real: {
            object = cos_obj_parser_handle_real_(parser, &token, error);
        } break;

        case CosToken_Type_ArrayStart: {
            object = cos_obj_parser_handle_array_(parser, error);
        } break;
        case CosToken_Type_ArrayEnd:
            break;

        case CosToken_Type_DictionaryStart: {
            object = cos_obj_parser_handle_dictionary_(parser, error);
        } break;
        case CosToken_Type_DictionaryEnd:
            break;

        case CosToken_Type_Keyword: {
            object = cos_obj_parser_handle_keyword_(parser, &token, error);
        } break;

        case CosToken_Type_EOF:
            break;
    }

    return object;
}

static CosBaseObj *
cos_obj_parser_handle_literal_string_(CosObjParser *parser,
                                      CosToken *token,
                                      CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);

    CosData *string_data = NULL;
    if (cos_token_value_take_data(&(token->value),
                                  &string_data)) {
        CosBaseObj * const string_object = (CosBaseObj *)cos_string_obj_alloc(string_data,
                                                                              parser->doc);
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
cos_obj_parser_handle_hex_string_(CosObjParser *parser,
                                  CosToken *token,
                                  CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);

    CosData *string_data = NULL;
    if (cos_token_value_take_data(&(token->value),
                                  &string_data)) {
        CosBaseObj * const string_object = (CosBaseObj *)cos_string_obj_alloc(string_data,
                                                                              parser->doc);
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
cos_obj_parser_handle_name_(CosObjParser *parser,
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
cos_obj_parser_handle_integer_(CosObjParser *parser,
                               const CosToken *token,
                               CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    CosIntegerObj *int_obj = NULL;

    // Get the integer value of the token.
    int int_value = 0;
    if (!cos_token_value_get_integer_number(&(token->value),
                                            &int_value)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid integer token"),
                            error);
        return NULL;
    }

    if (int_value < 0) {
        // We know that this integer is negative, so all the integers in the queue
        // and this integer should be treated as number objects.
        int_obj = cos_integer_obj_alloc(int_value);
        if (!int_obj) {
            return NULL;
        }

        const size_t integer_count = parser->integer_count;
        for (size_t i = 0; i < integer_count; i++) {
            const unsigned int queued_int = cos_obj_parser_peek_int_(parser);

            CosIntegerObj * const queued_int_object = cos_integer_obj_alloc((int)queued_int);
            if (!queued_int_object) {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                                   "Failed to allocate integer object"),
                                    error);
                // We have a local integer object that needs to be released.
                goto failure;
            }
            cos_obj_parser_pop_int_(parser);
            cos_obj_parser_push_object_(parser, (CosBaseObj *)queued_int_object);
        }

        // Add this integer's object to the end of the queue.
        cos_obj_parser_push_object_(parser, (CosBaseObj *)int_obj);

        // Return the integer object from the front of the queue.
        return cos_obj_parser_pop_object_(parser);
    }
    else {
        if (parser->integer_count > 1) {
            const int peeked_int = (int)cos_obj_parser_peek_int_(parser);
            // TODO: Only pop the integer after the object has been successfully allocated.
            int_obj = cos_integer_obj_alloc(peeked_int);
            if (!int_obj) {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                                   "Failed to allocate integer object"),
                                    error);
                goto failure;
            }

            cos_obj_parser_pop_int_(parser);
        }

        cos_obj_parser_push_int_(parser, (unsigned int)int_value);

        if (parser->integer_count >= 3) {
            const int popped_integer = (int)cos_obj_parser_pop_int_(parser);

            return (CosBaseObj *)cos_integer_obj_alloc(popped_integer);
        }

        return cos_obj_parser_next_object_(parser, error);
    }

failure:
    if (int_obj) {
        cos_obj_release((CosObj *)int_obj);
    }

    return NULL;
}

static CosBaseObj *
cos_obj_parser_handle_real_(CosObjParser *parser,
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
                                                parser->doc);
    }
    else {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid real token"),
                            error);
        return NULL;
    }
}

static CosBaseObj *
cos_obj_parser_handle_array_(CosObjParser *parser,
                             CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    CosArrayObj * const array_object = cos_array_obj_create();
    if (!array_object) {
        return NULL;
    }

    while (cos_tokenizer_has_next_token(parser->tokenizer)) {
        // Check for array end.
        if (cos_tokenizer_match_token(parser->tokenizer,
                                      CosToken_Type_ArrayEnd)) {
            break;
        }

        // Parse the next object.
        CosBaseObj * const object = cos_obj_parser_next_object_(parser, error);
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
cos_obj_parser_handle_dictionary_(CosObjParser *parser,
                                  CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    CosDictObj * const dict_obj = cos_dict_obj_create();
    if (!dict_obj) {
        goto failure;
    }

    while (cos_tokenizer_has_next_token(parser->tokenizer)) {
        // Check for dictionary end.
        if (cos_tokenizer_match_token(parser->tokenizer,
                                      CosToken_Type_DictionaryEnd)) {
            break;
        }

        // Parse the next object.
        CosBaseObj * const key = cos_obj_parser_next_object_(parser,
                                                             error);
        if (!key) {
            goto failure;
        }

        // TODO: Check that the key is a name.

        // Parse the next object.
        CosBaseObj * const value = cos_obj_parser_next_object_(parser,
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

static CosBaseObj *
cos_obj_parser_handle_keyword_(CosObjParser *parser,
                               const CosToken *token,
                               CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(token != NULL && token->type == CosToken_Type_Keyword);
    if (!parser || !token) {
        return NULL;
    }

    CosKeywordType keyword_type;
    if (!cos_token_value_get_keyword(&(token->value),
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
            return cos_obj_parser_handle_indirect_obj_ref_(parser, error);
        }
        case CosKeywordType_Obj: {
            return cos_obj_parser_handle_indirect_obj_def_(parser, error);
        }

        case CosKeywordType_EndObj:
            break;
        case CosKeywordType_Stream:
            break;
        case CosKeywordType_EndStream:
            break;
        case CosKeywordType_XRef:
        case CosKeywordType_Trailer:
        case CosKeywordType_StartXRef:
            break;
    }

    return NULL;
}

static CosBaseObj * COS_Nullable
cos_obj_parser_handle_indirect_obj_def_(CosObjParser *parser,
                                        CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    // TODO: We should have two integers on the stack.

    // The value of the indirect object follows the "obj" keyword.
    CosBaseObj * const obj = cos_obj_parser_next_object_(parser, error);
    if (!obj) {
        cos_diagnose(parser->diagnostic_handler,
                     CosDiagnosticLevel_Error,
                     "Invalid indirect object definition");
        return NULL;
    }

    cos_tokenizer_match_keyword(parser->tokenizer,
                                CosKeywordType_Unknown);

    if (!cos_tokenizer_match_keyword(parser->tokenizer,
                                     CosKeywordType_EndObj)) {
        cos_diagnose(parser->diagnostic_handler,
                     CosDiagnosticLevel_Warning,
                     "Unterminated indirect object definition");
    }

    return obj;
}

static CosBaseObj * COS_Nullable
cos_obj_parser_handle_indirect_obj_ref_(CosObjParser *parser,
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

    unsigned int object_number = cos_obj_parser_pop_int_(parser);
    unsigned int generation_number = cos_obj_parser_pop_int_(parser);

    const CosObjID obj_id = cos_obj_id_make(object_number,
                                            generation_number);

    return cos_doc_get_object(parser->doc, obj_id, error);
}

static void
cos_obj_parser_push_int_(CosObjParser *parser,
                         unsigned int integer)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return;
    }

    if (parser->integer_count == COS_ARRAY_SIZE(parser->integers)) {
        return;
    }

    parser->integers[parser->integer_count] = integer;
    parser->integer_count++;
}

static unsigned int
cos_obj_parser_peek_int_(const CosObjParser *parser)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return 0;
    }

    if (parser->integer_count == 0) {
        return 0;
    }

    return parser->integers[0];
}

static unsigned int
cos_obj_parser_pop_int_(CosObjParser *parser)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return 0;
    }

    if (parser->integer_count == 0) {
        return 0;
    }

    const unsigned int integer = parser->integers[0];

    memmove(parser->integers,
            parser->integers + 1,
            COS_ARRAY_SIZE(parser->integers) - 1);

    parser->integer_count--;

    return integer;
}

static void
cos_obj_parser_push_object_(CosObjParser *parser,
                            CosBaseObj *object)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser || !object) {
        return;
    }

    if (parser->object_count == COS_ARRAY_SIZE(parser->objects)) {
        return;
    }

    parser->objects[parser->object_count] = object;
    parser->object_count++;
}

static CosBaseObj *
cos_obj_parser_pop_object_(CosObjParser *parser)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    if (parser->object_count == 0) {
        return NULL;
    }

    CosBaseObj * const object = parser->objects[0];

    memmove(parser->objects,
            parser->objects + 1,
            COS_ARRAY_SIZE(parser->objects) - 1);

    parser->object_count--;

    return object;
}

COS_ASSUME_NONNULL_END
