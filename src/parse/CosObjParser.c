/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "CosObjParser.h"

#include "common/Assert.h"
#include "libcos/CosDoc.h"
#include "libcos/common/CosMacros.h"
#include "libcos/objects/CosArrayObj.h"
#include "libcos/objects/CosBoolObj.h"
#include "libcos/objects/CosIndirectObj.h"
#include "libcos/objects/CosIntObj.h"
#include "libcos/objects/CosNameObj.h"
#include "libcos/objects/CosNullObj.h"
#include "libcos/objects/CosObj.h"
#include "libcos/objects/CosRealObj.h"
#include "libcos/objects/CosReferenceObj.h"
#include "libcos/objects/CosStringObj.h"
#include "parse/CosTokenizer.h"
#include "parse/tokenizer/CosToken.h"

#include <libcos/CosObjID.h>
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

    CosObj * COS_Nullable objects[3];
    size_t object_count;

    CosDiagnosticHandler *diagnostic_handler;
};

static bool
cos_obj_parser_init_(CosObjParser *self,
                     CosDoc *document,
                     CosInputStream *input_stream);

static CosObj * COS_Nullable
cos_obj_parser_next_object_(CosObjParser *parser,
                            CosError * COS_Nullable error);

static CosObj * COS_Nullable
cos_obj_parser_handle_literal_string_(CosObjParser *parser,
                                      CosToken *token,
                                      CosError * COS_Nullable error);

static CosObj * COS_Nullable
cos_obj_parser_handle_hex_string_(CosObjParser *parser,
                                  CosToken *token,
                                  CosError * COS_Nullable error);

static CosObj * COS_Nullable
cos_obj_parser_handle_name_(CosObjParser *parser,
                            CosTokenValue *token_value,
                            CosError * COS_Nullable error);

static CosObj * COS_Nullable
cos_obj_parser_handle_integer_(CosObjParser *parser,
                               CosTokenValue *token_value,
                               CosError * COS_Nullable error);

static CosObj * COS_Nullable
cos_obj_parser_handle_real_(CosObjParser *parser,
                            const CosToken *token,
                            CosError * COS_Nullable error);

static CosObj * COS_Nullable
cos_obj_parser_handle_array_(CosObjParser *parser,
                             CosError * COS_Nullable error);

static CosObj * COS_Nullable
cos_obj_parser_handle_dictionary_(CosObjParser *parser,
                                  CosError * COS_Nullable error);

static CosObj * COS_Nullable
cos_obj_parser_handle_keyword_(CosObjParser *parser,
                               const CosTokenValue *token_value,
                               CosError * COS_Nullable error);

static CosObj * COS_Nullable
cos_obj_parser_handle_indirect_obj_def_(CosObjParser *parser,
                                        CosError * COS_Nullable error);

static CosObj * COS_Nullable
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
                            CosObj *object);

static CosObj * COS_Nullable
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
    if (!self || !document || !input_stream) {
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

    CosDiagnosticHandler *diagnostic_handler = cos_doc_get_diagnostic_handler(document);
    if (diagnostic_handler) {
        self->diagnostic_handler = diagnostic_handler;
    }
    else {
        self->diagnostic_handler = cos_diagnostic_handler_get_default();
    }

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
        CosObj * const obj = parser->objects[i];
        cos_obj_free(obj);
    }

    free(parser);
}

bool
cos_obj_parser_has_next_object(CosObjParser *parser)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return false;
    }

    const CosObj * const obj = cos_obj_parser_peek_object(parser, NULL);
    return (obj != NULL);
}

CosObj *
cos_obj_parser_peek_object(CosObjParser *parser,
                           CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    // If there are objects in the queue, return the first one.
    if (parser->object_count > 0) {
        return parser->objects[0];
    }

    // Otherwise, parse the next obj and push it to the queue.
    CosError error_ = cos_error_none();
    CosObj * const obj = cos_obj_parser_next_object_(parser, &error_);
    if (!obj) {
        COS_ERROR_PROPAGATE(error_, error);
        return NULL;
    }

    cos_obj_parser_push_object_(parser, obj);
    return obj;
}

CosObj *
cos_obj_parser_next_object(CosObjParser *parser,
                           CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    if (parser->object_count > 0) {
        return cos_obj_parser_pop_object_(parser);
    }

    return cos_obj_parser_next_object_(parser, error);
}

#pragma mark - Implementation

static CosObj *
cos_obj_parser_next_object_(CosObjParser *parser,
                            CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    if (parser->object_count > 0) {
        return cos_obj_parser_pop_object_(parser);
    }

    CosToken peeked_token = {0};
    const CosTokenValue *peeked_token_value = NULL;
    if (!cos_tokenizer_peek_next_token(parser->tokenizer,
                                       &peeked_token,
                                       &peeked_token_value,
                                       error)) {
        return NULL;
    }

    CosObj *object = NULL;

    // Deal with integers first.
    if (peeked_token.type == CosToken_Type_Integer) {
        peeked_token = cos_tokenizer_next_token(parser->tokenizer);
        return cos_obj_parser_handle_integer_(parser, (CosTokenValue *)peeked_token_value, error);
    }
    else if (peeked_token.type == CosToken_Type_Keyword) {
        CosKeywordType keyword_type;
        if (cos_token_value_get_keyword(peeked_token_value,
                                        &keyword_type)) {
            if (keyword_type == CosKeywordType_R) {
                peeked_token = cos_tokenizer_next_token(parser->tokenizer);
                return cos_obj_parser_handle_indirect_obj_ref_(parser, error);
            }
            else if (keyword_type == CosKeywordType_Obj) {
                peeked_token = cos_tokenizer_next_token(parser->tokenizer);
                return cos_obj_parser_handle_indirect_obj_def_(parser, error);
            }
        }
    }

    if (parser->integer_count > 0) {
        const unsigned int peeked_int = cos_obj_parser_peek_int_(parser);

        CosIntObj * const int_obj = cos_int_obj_alloc((int)peeked_int);
        if (!int_obj) {
            return NULL;
        }

        cos_obj_parser_pop_int_(parser);

        return (CosObj *)int_obj;
    }

    CosToken token = {0};
    CosTokenValue *token_value = NULL;

    if (!cos_tokenizer_get_next_token(parser->tokenizer,
                                      &token,
                                      &token_value,
                                      error)) {
        return NULL;
    }

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
            object = cos_obj_parser_handle_name_(parser, token_value, error);
        } break;

        case CosToken_Type_Integer: {
            object = cos_obj_parser_handle_integer_(parser, token_value, error);
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
            object = cos_obj_parser_handle_keyword_(parser, token_value, error);
        } break;

        case CosToken_Type_EOF:
            break;
    }

    return object;
}

static CosObj *
cos_obj_parser_handle_literal_string_(CosObjParser *parser,
                                      CosToken *token,
                                      CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(token != NULL && token->type == CosToken_Type_Literal_String);
    if (!parser || !token) {
        return NULL;
    }

    CosData *string_data = NULL;
    if (cos_token_value_take_data(token->value,
                                  &string_data)) {
        CosObj * const string_object = (CosObj *)cos_string_obj_alloc(string_data);
        return string_object;
    }
    else {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid literal string token"),
                            error);
    }

    return NULL;
}

static CosObj *
cos_obj_parser_handle_hex_string_(CosObjParser *parser,
                                  CosToken *token,
                                  CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(token != NULL && token->type == CosToken_Type_Hex_String);
    if (!parser || !token) {
        return NULL;
    }

    CosData *string_data = NULL;
    if (cos_token_value_take_data(token->value,
                                  &string_data)) {
        return (CosObj *)cos_string_obj_alloc(string_data);
    }
    else {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid hexadecimal string token"),
                            error);
    }

    return NULL;
}

static CosObj *
cos_obj_parser_handle_name_(CosObjParser *parser,
                            CosTokenValue *token_value,
                            CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(token_value != NULL);
    if (!parser || !token_value) {
        return NULL;
    }

    CosString *name = NULL;
    if (cos_token_value_take_string(token_value, &name)) {
        return (CosObj *)cos_name_obj_alloc(name);
    }
    else {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid name token"),
                            error);
        return NULL;
    }
}

static CosObj *
cos_obj_parser_handle_integer_(CosObjParser *parser,
                               CosTokenValue *token_value,
                               CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    CosIntObj *int_obj = NULL;

    // Get the integer value of the token.
    int int_value = 0;
    if (!cos_token_value_get_integer_number(token_value,
                                            &int_value)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid integer token"),
                            error);
        return NULL;
    }

    if (int_value < 0) {
        // We know that this integer is negative, so all the integers in the queue
        // and this integer should be treated as number objects.
        int_obj = cos_int_obj_alloc(int_value);
        if (!int_obj) {
            return NULL;
        }

        const size_t integer_count = parser->integer_count;
        for (size_t i = 0; i < integer_count; i++) {
            const unsigned int queued_int = cos_obj_parser_peek_int_(parser);

            CosIntObj * const queued_int_object = cos_int_obj_alloc((int)queued_int);
            if (!queued_int_object) {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                                   "Failed to allocate integer object"),
                                    error);
                // We have a local integer object that needs to be released.
                goto failure;
            }
            cos_obj_parser_pop_int_(parser);
            cos_obj_parser_push_object_(parser, (CosObj *)queued_int_object);
        }

        // Add this integer's object to the end of the queue.
        cos_obj_parser_push_object_(parser, (CosObj *)int_obj);

        // Return the integer object from the front of the queue.
        return cos_obj_parser_pop_object_(parser);
    }
    else {
        if (parser->integer_count > 1) {
            const int peeked_int = (int)cos_obj_parser_peek_int_(parser);
            int_obj = cos_int_obj_alloc(peeked_int);
            if (!int_obj) {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                                   "Failed to allocate integer object"),
                                    error);
                goto failure;
            }

            // Only pop the integer after the object has been successfully allocated.
            cos_obj_parser_pop_int_(parser);
        }

        cos_obj_parser_push_int_(parser, (unsigned int)int_value);

        if (int_obj) {
            return (CosObj *)int_obj;
        }
        else if (parser->integer_count >= 3) {
            const int popped_integer = (int)cos_obj_parser_pop_int_(parser);

            return (CosObj *)cos_int_obj_alloc(popped_integer);
        }

        return cos_obj_parser_next_object_(parser, error);
    }

failure:
    if (int_obj) {
        free(int_obj);
    }

    return NULL;
}

static CosObj *
cos_obj_parser_handle_real_(CosObjParser *parser,
                            const CosToken *token,
                            CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(token != NULL && token->type == CosToken_Type_Real);
    if (!parser || !token) {
        return NULL;
    }

    double real_value = 0.0;
    if (cos_token_value_get_real_number(token->value,
                                        &real_value)) {
        return (CosObj *)cos_real_obj_alloc(real_value);
    }
    else {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid real token"),
                            error);
        return NULL;
    }
}

static CosObj *
cos_obj_parser_handle_array_(CosObjParser *parser,
                             CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    CosArrayObj * const array_object = cos_array_obj_alloc(NULL);
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
        CosObj * const object = cos_obj_parser_next_object_(parser, error);
        if (!object) {
            goto failure;
        }

        // Add the object to the array.
        if (!cos_array_obj_append(array_object,
                                  (void *)object,
                                  error)) {
            goto failure;
        }
    }

    return (CosObj *)array_object;

failure:
    return NULL;
}

static CosObj *
cos_obj_parser_handle_dictionary_(CosObjParser *parser,
                                  CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    CosDictObj * const dict_obj = cos_dict_obj_alloc(NULL);
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
        CosObj * const key = cos_obj_parser_next_object_(parser,
                                                         error);
        if (!key) {
            goto failure;
        }

        cos_obj_print_desc(key);

        // TODO: Check that the key is a name.

        // Parse the next object.
        CosObj * const value = cos_obj_parser_next_object_(parser,
                                                           error);
        if (!value) {
            goto failure;
        }

        // Add the object to the dictionary.
        if (!cos_dict_obj_set(dict_obj,
                              (CosNameObj *)key,
                              (CosObj *)value,
                              error)) {
            goto failure;
        }
    }

    return (CosObj *)dict_obj;

failure:
    return NULL;
}

static CosObj *
cos_obj_parser_handle_keyword_(CosObjParser *parser,
                               const CosTokenValue *token_value,
                               CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(token_value != NULL);
    if (!parser || !token_value) {
        return NULL;
    }

    CosKeywordType keyword_type;
    if (!cos_token_value_get_keyword(token_value,
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
            return (CosObj *)cos_bool_obj_alloc(true);
        }
        case CosKeywordType_False: {
            return (CosObj *)cos_bool_obj_alloc(false);
        }

        case CosKeywordType_Null: {
            return (CosObj *)cos_null_obj_get();
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
        case CosKeywordType_N:
        case CosKeywordType_F:
        case CosKeywordType_Trailer:
        case CosKeywordType_StartXRef:
            break;
    }

    return NULL;
}

static CosObjID
cos_parser_get_obj_id_(CosObjParser *parser,
                       CosError * COS_Nullable error)
{
    if (parser->integer_count >= 2) {
        const unsigned int obj_number = cos_obj_parser_pop_int_(parser);
        const unsigned int gen_number = cos_obj_parser_pop_int_(parser);
        return cos_obj_id_make(obj_number, gen_number);
    }
    else if (parser->integer_count == 1) {
        cos_diagnose(parser->diagnostic_handler,
                     CosDiagnosticLevel_Warning,
                     "Indirect object reference missing generation number");

        const unsigned int obj_number = cos_obj_parser_pop_int_(parser);
        return cos_obj_id_make(obj_number, 0);
    }
    else {
        cos_diagnose(parser->diagnostic_handler,
                     CosDiagnosticLevel_Error,
                     "Invalid indirect object reference");

        return CosObjID_Invalid;
    }
}

static CosObj * COS_Nullable
cos_obj_parser_handle_indirect_obj_def_(CosObjParser *parser,
                                        CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    const CosObjID obj_id = cos_parser_get_obj_id_(parser, error);
    if (!cos_obj_id_is_valid(obj_id)) {
        cos_diagnose(parser->diagnostic_handler,
                     CosDiagnosticLevel_Error,
                     "Invalid indirect object definition");
        return NULL;
    }

    // The value of the indirect object follows the "obj" keyword.
    CosObj * const obj = cos_obj_parser_next_object_(parser, error);
    if (!obj) {
        cos_diagnose(parser->diagnostic_handler,
                     CosDiagnosticLevel_Error,
                     "Invalid indirect object definition");
        return NULL;
    }

    if (!cos_tokenizer_match_keyword(parser->tokenizer,
                                     CosKeywordType_EndObj)) {
        cos_diagnose(parser->diagnostic_handler,
                     CosDiagnosticLevel_Warning,
                     "Unterminated indirect object definition");
    }

    CosIndirectObj * const indirect_obj = cos_indirect_obj_alloc(obj_id,
                                                                 obj);
    if (!indirect_obj) {
        return obj;
    }

    return (CosObj *)indirect_obj;
}

static CosObj * COS_Nullable
cos_obj_parser_handle_indirect_obj_ref_(CosObjParser *parser,
                                        CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    const CosObjID obj_id = cos_parser_get_obj_id_(parser, error);
    if (!cos_obj_id_is_valid(obj_id)) {
        cos_diagnose(parser->diagnostic_handler,
                     CosDiagnosticLevel_Error,
                     "Invalid indirect object reference"); // TODO: Include error.
        return NULL;
    }

    return (CosObj *)cos_reference_obj_alloc(obj_id,
                                             parser->doc);
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
                            CosObj *object)
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

static CosObj *
cos_obj_parser_pop_object_(CosObjParser *parser)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    if (parser->object_count == 0) {
        return NULL;
    }

    CosObj * const object = parser->objects[0];

    memmove(parser->objects,
            parser->objects + 1,
            COS_ARRAY_SIZE(parser->objects) - 1);

    parser->object_count--;

    return object;
}

COS_ASSUME_NONNULL_END
