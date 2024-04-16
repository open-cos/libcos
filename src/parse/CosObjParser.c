/*
 * Copyright (c) 2023-2024 OpenCOS.
 */

#include "CosObjParser.h"

#include "common/Assert.h"
#include "syntax/tokenizer/CosToken.h"
#include "syntax/tokenizer/CosTokenizer.h"

#include <libcos/CosDoc.h>
#include <libcos/CosObjID.h>
#include <libcos/common/CosDiagnosticHandler.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosRingBuffer.h>
#include <libcos/io/CosInputStream.h>
#include <libcos/objects/CosArrayObj.h>
#include <libcos/objects/CosBoolObj.h>
#include <libcos/objects/CosDictObj.h>
#include <libcos/objects/CosIndirectObj.h>
#include <libcos/objects/CosIntObj.h>
#include <libcos/objects/CosNameObj.h>
#include <libcos/objects/CosNullObj.h>
#include <libcos/objects/CosObj.h>
#include <libcos/objects/CosRealObj.h>
#include <libcos/objects/CosReferenceObj.h>
#include <libcos/objects/CosStringObj.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosObjParser {
    CosDoc *doc;
    CosTokenizer *tokenizer;

    CosRingBuffer *objects;
    CosRingBuffer *integers;

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
cos_handle_integer_(CosObjParser *parser,
                    CosTokenValue *token_value,
                    CosError * COS_Nullable out_error);

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

static CosIndirectObj * COS_Nullable
cos_handle_indirect_obj_def_(CosObjParser *parser,
                             CosError * COS_Nullable out_error);

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
cos_obj_parser_create(CosDoc *document,
                      CosInputStream *input_stream)
{
    COS_PARAMETER_ASSERT(document != NULL);
    COS_PARAMETER_ASSERT(input_stream != NULL);
    if (!input_stream) {
        return NULL;
    }

    CosObjParser * const parser = calloc(1, sizeof(CosObjParser));
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
cos_obj_parser_init_(CosObjParser * const self,
                     CosDoc *document,
                     CosInputStream *input_stream)
{
    COS_PARAMETER_ASSERT(self != NULL);
    COS_PARAMETER_ASSERT(document != NULL);
    COS_PARAMETER_ASSERT(input_stream != NULL);
    if (!self || !document || !input_stream) {
        return false;
    }

    CosRingBuffer *integers = NULL;
    CosRingBuffer *objects = NULL;

    CosTokenizer * const tokenizer = cos_tokenizer_alloc(input_stream);
    if (!tokenizer) {
        goto failure;
    }

    self->doc = document;
    self->tokenizer = tokenizer;

    integers = cos_ring_buffer_create(sizeof(unsigned int),
                                      2);
    if (!integers) {
        goto failure;
    }

    objects = cos_ring_buffer_create(sizeof(CosObj *),
                                     3);
    if (!objects) {
        goto failure;
    }

    CosDiagnosticHandler *diagnostic_handler = cos_doc_get_diagnostic_handler(document);
    if (diagnostic_handler) {
        self->diagnostic_handler = diagnostic_handler;
    }
    else {
        self->diagnostic_handler = cos_diagnostic_handler_get_default();
    }

    self->integers = integers;
    self->objects = objects;

    return true;

failure:
    if (tokenizer) {
        cos_tokenizer_free(tokenizer);
    }
    if (integers) {
        cos_ring_buffer_destroy(integers);
    }
    if (objects) {
        cos_ring_buffer_destroy(objects);
    }
    return false;
}

void
cos_obj_parser_destroy(CosObjParser *parser)
{
    if (!parser) {
        return;
    }

    cos_tokenizer_free(parser->tokenizer);

    cos_ring_buffer_destroy(parser->integers);
    cos_ring_buffer_destroy(parser->objects);

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
    CosObj *peeked_obj = NULL;
    if (cos_ring_buffer_get_first_item(parser->objects,
                                       &peeked_obj)) {
        return peeked_obj;
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

    CosObj *popped_obj = NULL;
    if (cos_ring_buffer_pop_front(parser->objects,
                                  &popped_obj)) {
        return popped_obj;
    }

    return cos_obj_parser_next_object_(parser, error);
}

#pragma mark - Implementation

// NOLINTBEGIN(misc-no-recursion)

static CosObj *
cos_obj_parser_next_object_(CosObjParser *parser,
                            CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    // Check if there are peeked objects in the queue.
    CosObj *popped_obj = NULL;
    if (cos_ring_buffer_pop_front(parser->objects,
                                  &popped_obj)) {
        return popped_obj;
    }

    CosObj *object = NULL;

    CosToken *token = cos_tokenizer_get_next_token(parser->tokenizer,
                                                   error);
    if (!token) {
        return NULL;
    }

    switch (token->type) {
        case CosToken_Type_Unknown:
            break;

        case CosToken_Type_Literal_String: {
            object = cos_obj_parser_handle_literal_string_(parser, token, error);
        } break;
        case CosToken_Type_Hex_String: {
            object = cos_obj_parser_handle_hex_string_(parser, token, error);
        } break;

        case CosToken_Type_Name: {
            object = cos_obj_parser_handle_name_(parser, token->value, error);
        } break;

        case CosToken_Type_Integer: {
            object = cos_handle_integer_(parser, token->value, error);
        } break;
        case CosToken_Type_Real: {
            object = cos_obj_parser_handle_real_(parser, token, error);
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
            object = cos_obj_parser_handle_keyword_(parser, token->value, error);
        } break;

        case CosToken_Type_EOF:
            break;
    }

    cos_tokenizer_release_token(parser->tokenizer,
                                token);

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
cos_handle_integer_(CosObjParser *parser,
                    CosTokenValue *token_value,
                    CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(token_value != NULL);
    if (!parser || !token_value) {
        return NULL;
    }

    // Get the integer value of the token.
    int int_value = 0;
    if (!cos_token_value_get_integer_number(token_value,
                                            &int_value)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid integer token"),
                            out_error);
        goto failure;
    }

    const CosToken *peeked_token = cos_tokenizer_peek_next_token(parser->tokenizer,
                                                                 out_error);

    CosToken peeked_next_token = {0};
    const CosTokenValue *peeked_next_token_value = NULL;

    if (!peeked_token ||
        !cos_tokenizer_peek_next_next_token(parser->tokenizer,
                                            &peeked_next_token,
                                            &peeked_next_token_value,
                                            out_error)) {
        goto integer_number;
    }

    if (peeked_token->type == CosToken_Type_Integer &&
        peeked_next_token.type == CosToken_Type_Keyword) {
        // This could be an indirect object definition or reference.
        // Check the keyword type to determine which one it is.
        CosKeywordType keyword_type = CosKeywordType_Unknown;
        if (!cos_token_value_get_keyword(peeked_next_token_value,
                                         &keyword_type)) {
            // Invalid keyword token - ignored.
            goto integer_number;
        }

        switch (keyword_type) {
            case CosKeywordType_Obj: {
                // This is an indirect object definition.
                return cos_obj_parser_handle_indirect_obj_def_(parser, out_error);
            }
            case CosKeywordType_R: {
                // This is an indirect object reference.
                return cos_obj_parser_handle_indirect_obj_ref_(parser, out_error);
            }

            case CosKeywordType_Unknown:
            case CosKeywordType_True:
            case CosKeywordType_False:
            case CosKeywordType_Null:
            case CosKeywordType_EndObj:
            case CosKeywordType_Stream:
            case CosKeywordType_EndStream:
            case CosKeywordType_XRef:
            case CosKeywordType_N:
            case CosKeywordType_F:
            case CosKeywordType_Trailer:
            case CosKeywordType_StartXRef: {
                goto integer_number;
            }
        }
    }

integer_number:
    // This is just a regular integer.
    return (CosObj *)cos_int_obj_alloc(int_value);

failure:
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
                                  object,
                                  error)) {
            goto failure;
        }
    }

    return (CosObj *)array_object;

failure:
    // We still return the array object even if an error occurred.
    return (CosObj *)array_object;
}

static CosObj *
cos_obj_parser_handle_dictionary_(CosObjParser *parser,
                                  CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    CosDictObj *dict_obj = NULL;
    CosObj *key = NULL;
    CosObj *value = NULL;

    dict_obj = cos_dict_obj_create(NULL);
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
        key = cos_obj_parser_next_object_(parser,
                                          error);
        if (!key) {
            goto failure;
        }

        cos_obj_print_desc(key);

        // TODO: Check that the key is a name.

        // Parse the next object.
        value = cos_obj_parser_next_object_(parser,
                                            error);
        if (!value) {
            goto failure;
        }

        // Add the object to the dictionary.
        if (!cos_dict_obj_set(dict_obj,
                              (CosNameObj *)key,
                              value,
                              error)) {
            goto failure;
        }

        // Clear the key and value for the next iteration.
        key = NULL;
        value = NULL;
    }

    return (CosObj *)dict_obj;

failure:
    if (key) {
        cos_obj_free(key);
    }
    if (value) {
        cos_obj_free(value);
    }

    // We still return the dictionary object even if an error occurred.
    return (CosObj *)dict_obj;
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
                       CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(parser != NULL);

    const size_t integer_count = cos_ring_buffer_get_count(parser->integers);
    if (integer_count >= 2) {
        const unsigned int obj_number = cos_obj_parser_pop_int_(parser);
        const unsigned int gen_number = cos_obj_parser_pop_int_(parser);
        return cos_obj_id_make(obj_number, gen_number);
    }
    else if (integer_count == 1) {
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

    printf("Obj ID: %d %d\n", obj_id.obj_number, obj_id.gen_number);

    // ISO 19005-1:2005(E), Section 6.1.8 Indirect objects (PDF/A-1a, PDF/A-1b)
    // "The object number and generation number shall be separated by a single white-space character."
    // "The generation number and the obj keyword shall be separated by a single white-space character."
    // "The object number and endobj keyword shall each be preceded by an EOL marker."
    // "The obj and endobj keywords shall each be followed by an EOL marker.

    if (!cos_tokenizer_match_keyword(parser->tokenizer,
                                     CosKeywordType_Obj)) {
        cos_diagnose(parser->diagnostic_handler,
                     CosDiagnosticLevel_Error,
                     "Invalid indirect object definition: missing 'obj' keyword");
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

static CosIndirectObj *
cos_handle_indirect_obj_def_(CosObjParser *parser,
                             CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    return NULL;
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

// NOLINTEND(misc-no-recursion)

static void
cos_obj_parser_push_int_(CosObjParser *parser,
                         unsigned int integer)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return;
    }

    cos_ring_buffer_push_back(parser->integers,
                              &integer,
                              NULL);
}

static unsigned int
cos_obj_parser_peek_int_(const CosObjParser *parser)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return 0;
    }

    unsigned int result = 0;
    cos_ring_buffer_get_first_item(parser->integers,
                                   &result);
    return result;
}

static unsigned int
cos_obj_parser_pop_int_(CosObjParser *parser)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return 0;
    }

    unsigned int result = 0;
    cos_ring_buffer_pop_front(parser->integers,
                              &result);
    return result;
}

static void
cos_obj_parser_push_object_(CosObjParser *parser,
                            CosObj *object)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser || !object) {
        return;
    }

    cos_ring_buffer_push_back(parser->objects,
                              &object,
                              NULL);
}

static CosObj *
cos_obj_parser_pop_object_(CosObjParser *parser)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    CosObj *obj = NULL;
    cos_ring_buffer_pop_front(parser->objects, &obj);
    return obj;
}

COS_ASSUME_NONNULL_END
