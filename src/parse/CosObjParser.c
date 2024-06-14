/*
 * Copyright (c) 2023-2024 OpenCOS.
 */

#include "CosObjParser.h"

#include "common/Assert.h"
#include "common/CosDict.h"
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
#include <libcos/objects/CosStreamObj.h>
#include <libcos/objects/CosStringObj.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

typedef enum CosObjParserFlags {
    CosObjParserFlag_None = 0,

    CosObjParserFlag_BoolObj = 1 << 0,
    CosObjParserFlag_IntObj = 1 << 1,
    CosObjParserFlag_RealObj = 1 << 2,
    CosObjParserFlag_StringObj = 1 << 3,
    CosObjParserFlag_NameObj = 1 << 4,
    CosObjParserFlag_ArrayObj = 1 << 5,
    CosObjParserFlag_DictObj = 1 << 6,
    CosObjParserFlag_NullObj = 1 << 7,
    CosObjParserFlag_StreamObj = 1 << 8,
    CosObjParserFlag_IndirectObjDef = 1 << 9,
    CosObjParserFlag_IndirectObjRef = 1 << 10,

    CosObjParserFlag_NumberObj = (CosObjParserFlag_IntObj |
                                  CosObjParserFlag_RealObj),

    CosObjParserFlag_DirectObj = (CosObjParserFlag_BoolObj |
                                  CosObjParserFlag_IntObj |
                                  CosObjParserFlag_RealObj |
                                  CosObjParserFlag_StringObj |
                                  CosObjParserFlag_NameObj |
                                  CosObjParserFlag_ArrayObj |
                                  CosObjParserFlag_DictObj |
                                  CosObjParserFlag_NullObj),

} COS_ATTR_FLAG_ENUM CosObjParserFlags;

typedef struct CosObjParserContext {
    CosObjParserFlags flags;
} CosObjParserContext;

struct CosObjParser {
    CosDoc *doc;
    CosTokenizer *tokenizer;

    CosRingBuffer *objects;

    CosToken * COS_Nullable next_token;

    CosDiagnosticHandler *diagnostic_handler;
};

static bool
cos_obj_parser_init_(CosObjParser *self,
                     CosDoc *document,
                     CosStream *input_stream);

static CosObj * COS_Nullable
cos_next_object_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 CosError * COS_Nullable out_error)
    COS_OWNERSHIP_RETURNS;

static CosObj * COS_Nullable
cos_handle_literal_string_(CosObjParser *parser,
                           CosToken *token,
                           CosError * COS_Nullable error);

static CosObj * COS_Nullable
cos_handle_hex_string_(CosObjParser *parser,
                       CosToken *token,
                       CosError * COS_Nullable error);

static CosObj * COS_Nullable
cos_handle_name_(CosObjParser *parser,
                 CosToken *token,
                 CosError * COS_Nullable error);

static CosObj * COS_Nullable
cos_handle_integer_or_indirect_(CosObjParser *parser,
                                const CosObjParserContext *context,
                                CosToken *token,
                                CosError * COS_Nullable out_error);

static CosObj * COS_Nullable
cos_handle_integer_(CosObjParser *parser,
                    const CosObjParserContext *context,
                    CosToken *token,
                    CosError * COS_Nullable out_error);

static CosObj * COS_Nullable
cos_handle_real_(CosObjParser *parser,
                 const CosToken *token,
                 CosError * COS_Nullable error);

static void
cos_obj_parser_push_object_(CosObjParser *parser,
                            CosObj *object);

static CosObj * COS_Nullable
cos_obj_parser_pop_object_(CosObjParser *parser);

static bool
cos_has_next_token_(CosObjParser *parser);

static const CosToken * COS_Nullable
cos_peek_next_token_(CosObjParser *parser,
                     CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(2);

static CosToken * COS_Nullable
cos_get_next_token_(CosObjParser *parser,
                    CosError * COS_Nullable out_error)
    COS_OWNERSHIP_RETURNS
    COS_ATTR_ACCESS_WRITE_ONLY(2);

static CosToken * COS_Nullable
cos_match_next_token_(CosObjParser *parser,
                      CosToken_Type type,
                      CosError * COS_Nullable out_error)
    COS_OWNERSHIP_RETURNS
    COS_ATTR_ACCESS_WRITE_ONLY(3);

static bool
cos_matches_next_token_(CosObjParser *parser,
                        CosToken_Type type,
                        CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

CosObjParser *
cos_obj_parser_create(CosDoc *document,
                      CosStream *input_stream)
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
                     CosStream *input_stream)
{
    COS_PARAMETER_ASSERT(self != NULL);
    COS_PARAMETER_ASSERT(document != NULL);
    COS_PARAMETER_ASSERT(input_stream != NULL);
    if (!self || !document || !input_stream) {
        return false;
    }

    CosRingBuffer *objects = NULL;

    CosTokenizer * const tokenizer = cos_tokenizer_alloc(input_stream);
    if (!tokenizer) {
        goto failure;
    }

    self->doc = document;
    self->tokenizer = tokenizer;

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

    self->objects = objects;

    return true;

failure:
    if (tokenizer) {
        cos_tokenizer_free(tokenizer);
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

    if (parser->next_token) {
        cos_tokenizer_release_token(parser->tokenizer,
                                    COS_nonnull_cast(parser->next_token));
    }
    cos_tokenizer_free(parser->tokenizer);

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
                                       (void *)&peeked_obj)) {
        return peeked_obj;
    }

    const CosObjParserContext context = {
        .flags = CosObjParserFlag_IndirectObjDef,
    };

    // Otherwise, parse the next obj and push it to the queue.
    CosError error_ = cos_error_none();
    CosObj * const obj = cos_next_object_(parser,
                                          &context,
                                          &error_);
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
                                  (void *)&popped_obj)) {
        return popped_obj;
    }

    const CosObjParserContext context = {
        .flags = CosObjParserFlag_IndirectObjDef,
    };

    return cos_next_object_(parser,
                            &context,
                            error);
}

#pragma mark - Implementation

// NOLINTBEGIN(misc-no-recursion)

static CosObj * COS_Nullable
cos_handle_array_(CosObjParser *parser,
                  const CosObjParserContext *context,
                  CosError * COS_Nullable out_error);

static bool
cos_handle_array_element_(CosObjParser *parser,
                          const CosObjParserContext *context,
                          CosArray *array,
                          CosError * COS_Nullable out_error);

static CosObj * COS_Nullable
cos_handle_dict_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 CosError * COS_Nullable out_error);

static bool
cos_handle_dict_entry_(CosObjParser *parser,
                       const CosObjParserContext *context,
                       CosDict *dict,
                       CosError * COS_Nullable out_error);

static CosObj * COS_Nullable
cos_handle_stream_(CosObjParser *parser,
                   const CosObjParserContext *context,
                   CosDictObj *dict_obj,
                   CosError * COS_Nullable out_error);

static CosObj * COS_Nullable
cos_handle_bool_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 bool value,
                 CosError * COS_Nullable out_error);

static CosObj * COS_Nullable
cos_handle_null_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 CosError * COS_Nullable out_error);

static CosObj * COS_Nullable
cos_handle_indirect_ref_(CosObjParser *parser,
                         const CosObjParserContext *context,
                         CosToken *obj_num_token,
                         CosToken *gen_num_token,
                         CosToken *token,
                         CosError * COS_Nullable out_error)
    COS_OWNERSHIP_TAKES(3, 4, 5);

static CosObj * COS_Nullable
cos_handle_indirect_def_(CosObjParser *parser,
                         const CosObjParserContext *context,
                         CosToken *obj_num_token,
                         CosToken *gen_num_token,
                         CosToken *token,
                         CosError * COS_Nullable out_error)
    COS_OWNERSHIP_TAKES(3, 4, 5);

static CosObj *
cos_next_object_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (COS_UNLIKELY(!parser)) {
        return NULL;
    }

    CosToken * const token = cos_get_next_token_(parser,
                                                 out_error);
    if (!token) {
        goto failure;
    }

    switch (token->type) {
        case CosToken_Type_Unknown:
            break;

        case CosToken_Type_Literal_String: {
            return cos_handle_literal_string_(parser, token, out_error);
        }
        case CosToken_Type_Hex_String: {
            return cos_handle_hex_string_(parser, token, out_error);
        }

        case CosToken_Type_Name: {
            return cos_handle_name_(parser, token, out_error);
        }

        case CosToken_Type_Integer: {
            return cos_handle_integer_or_indirect_(parser,
                                                   context,
                                                   token,
                                                   out_error);
        }
        case CosToken_Type_Real: {
            return cos_handle_real_(parser, token, out_error);
        }

        case CosToken_Type_ArrayStart: {
            return cos_handle_array_(parser,
                                     context,
                                     out_error);
        }
        case CosToken_Type_ArrayEnd:
            break;

        case CosToken_Type_DictionaryStart: {
            return cos_handle_dict_(parser,
                                    context,
                                    out_error);
        }
        case CosToken_Type_DictionaryEnd:
            break;

        case CosToken_Type_True: {
            return cos_handle_bool_(parser,
                                    context,
                                    true,
                                    out_error);
        }
        case CosToken_Type_False: {
            return cos_handle_bool_(parser,
                                    context,
                                    false,
                                    out_error);
        }
        case CosToken_Type_Null: {
            return cos_handle_null_(parser,
                                    context,
                                    out_error);
        }
        case CosToken_Type_R:
        case CosToken_Type_Obj:
            // Unexpected token.
            break;

        case CosToken_Type_EOF:
            break;
        case CosToken_Type_EndObj:
            break;
        case CosToken_Type_Stream:
            break;
        case CosToken_Type_EndStream:
            break;
        case CosToken_Type_XRef:
            break;
        case CosToken_Type_N:
            break;
        case CosToken_Type_F:
            break;
        case CosToken_Type_Trailer:
            break;
        case CosToken_Type_StartXRef:
            break;
    }

failure:
    if (token) {
        cos_tokenizer_release_token(parser->tokenizer,
                                    token);
    }
    return NULL;
}

static CosObj *
cos_handle_integer_or_indirect_(CosObjParser *parser,
                                const CosObjParserContext *context,
                                CosToken *token,
                                CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(context != NULL);
    COS_PARAMETER_ASSERT(token != NULL && token->type == CosToken_Type_Integer);
    if (COS_UNLIKELY(!parser || !context || !token)) {
        return NULL;
    }

    CosObj *obj = NULL;
    CosToken *second_token = NULL;

    // Is the next token another integer?
    second_token = cos_match_next_token_(parser,
                                         CosToken_Type_Integer,
                                         out_error);
    if (!second_token) {
        goto integer_number;
    }

    const CosToken * const peeked_token = cos_peek_next_token_(parser,
                                                               out_error);
    if (!peeked_token) {
        goto integer_number;
    }

    if (peeked_token->type == CosToken_Type_R) {
        // This is an indirect object reference.
        CosToken * const keyword_token = cos_get_next_token_(parser,
                                                             out_error);
        COS_ASSERT(keyword_token != NULL, "Expected a token");
        if (COS_UNLIKELY(!keyword_token)) {
            goto failure;
        }

        obj = cos_handle_indirect_ref_(parser,
                                       context,
                                       token,
                                       second_token,
                                       keyword_token,
                                       out_error);
        return obj;
    }
    else if (peeked_token->type == CosToken_Type_Obj) {
        // This is an indirect object definition.
        CosToken * const keyword_token = cos_get_next_token_(parser,
                                                             out_error);
        COS_ASSERT(keyword_token != NULL, "Expected a token");
        if (COS_UNLIKELY(!keyword_token)) {
            goto failure;
        }

        obj = cos_handle_indirect_def_(parser,
                                       context,
                                       token,
                                       second_token,
                                       keyword_token,
                                       out_error);
        return obj;
    }

integer_number:

    if (second_token) {
        COS_ASSERT(parser->next_token == NULL, "Invalid state");
        parser->next_token = second_token;

        second_token = NULL;
    }

    obj = cos_handle_integer_(parser,
                              context,
                              token,
                              out_error);
    return obj;

failure:
    if (second_token) {
        cos_tokenizer_release_token(parser->tokenizer,
                                    second_token);
    }
    return obj;
}

static CosObj *
cos_handle_integer_(CosObjParser *parser,
                    const CosObjParserContext *context,
                    CosToken *token,
                    CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(context != NULL);
    COS_PARAMETER_ASSERT(token != NULL && token->type == CosToken_Type_Integer);
    if (COS_UNLIKELY(!parser || !context || !token)) {
        return NULL;
    }

    // Get the integer value of the token.
    int int_value = 0;
    if (!cos_token_value_get_integer_number(token->value,
                                            &int_value)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid integer token"),
                            out_error);
        goto failure;
    }

    return (CosObj *)cos_int_obj_alloc(int_value);

failure:
    return NULL;
}

static CosObj *
cos_handle_array_(CosObjParser *parser,
                  const CosObjParserContext *context,
                  CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(context != NULL);
    if (COS_UNLIKELY(!parser || !context)) {
        return NULL;
    }

    CosArray *array = NULL;

    array = cos_array_create(sizeof(CosObj *),
                             NULL,
                             0);
    if (!array) {
        goto failure;
    }

    bool element_success = false;
    while (cos_has_next_token_(parser)) {
        // Parse the next array element.
        element_success = cos_handle_array_element_(parser,
                                                    context,
                                                    array,
                                                    out_error);
        if (!element_success) {
            // Or, skip element and continue parsing?
            break;
        }
    }

    CosArrayObj * const array_obj = cos_array_obj_alloc(array);
    if (!array_obj) {
        goto failure;
    }

    return (CosObj *)array_obj;

failure:
    if (array) {
        cos_array_destroy(array);
    }
    return NULL;
}

static bool
cos_handle_array_element_(CosObjParser *parser,
                          const CosObjParserContext *context,
                          CosArray *array,
                          CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(context != NULL);
    COS_PARAMETER_ASSERT(array != NULL);
    if (COS_UNLIKELY(!parser || !context || !array)) {
        return false;
    }

    CosToken * const token = cos_match_next_token_(parser,
                                                   CosToken_Type_ArrayEnd,
                                                   out_error);
    if (token) {
        cos_tokenizer_release_token(parser->tokenizer,
                                    token);
        return false;
    }

    CosObj *element = NULL;

    const CosObjParserContext element_context = {
        .flags = (CosObjParserFlag_DirectObj |
                  CosObjParserFlag_IndirectObjRef),
    };

    // Parse the next object.
    element = cos_next_object_(parser,
                               &element_context,
                               out_error);
    if (!element) {
        goto failure;
    }

    // Append the object to the array.
    const bool append_success = cos_array_append_item(array,
                                                      (void *)&element,
                                                      out_error);
    if (COS_UNLIKELY(!append_success)) {
        goto failure;
    }

    return true;

failure:
    if (element) {
        cos_obj_free(element);
    }
    return false;
}

static CosObj *
cos_handle_dict_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(context != NULL);
    if (COS_UNLIKELY(!parser || !context)) {
        return NULL;
    }

    CosDict *new_dict = NULL;

    new_dict = cos_dict_alloc(&cos_dict_obj_key_callbacks,
                              &cos_dict_obj_value_callbacks,
                              0);
    if (!new_dict) {
        goto failure;
    }

    bool entry_success = false;
    while (cos_has_next_token_(parser)) {
        entry_success = cos_handle_dict_entry_(parser,
                                               context,
                                               new_dict,
                                               out_error);
        if (!entry_success) {
            // Or, skip entry and continue parsing?
            break;
        }
    }

    CosDictObj * const dict_obj = cos_dict_obj_create(new_dict);
    if (!dict_obj) {
        goto failure;
    }

    // Check if the next token denotes the beginning of a stream object's data.
    if (cos_matches_next_token_(parser,
                                CosToken_Type_Stream,
                                out_error)) {
        return cos_handle_stream_(parser,
                                  context,
                                  dict_obj,
                                  out_error);
    }

    return (CosObj *)dict_obj;

failure:
    if (new_dict) {
        cos_dict_free(new_dict);
    }
    return NULL;
}

static bool
cos_handle_dict_entry_(CosObjParser *parser,
                       const CosObjParserContext *context,
                       CosDict *dict,
                       CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(context != NULL);
    COS_PARAMETER_ASSERT(dict != NULL);
    if (COS_UNLIKELY(!parser || !context || !dict)) {
        return false;
    }

    CosToken * const token = cos_match_next_token_(parser,
                                                   CosToken_Type_DictionaryEnd,
                                                   out_error);
    if (token) {
        cos_tokenizer_release_token(parser->tokenizer,
                                    token);
        return false;
    }

    CosObj *key = NULL;
    CosObj *value = NULL;

    const CosObjParserContext key_context = {
        .flags = CosObjParserFlag_NameObj,
    };

    const CosObjParserContext value_context = {
        .flags = (CosObjParserFlag_DirectObj |
                  CosObjParserFlag_IndirectObjRef),
    };

    // Parse the next object.
    key = cos_next_object_(parser,
                           &key_context,
                           out_error);
    if (!key) {
        goto failure;
    }

    value = cos_next_object_(parser,
                             &value_context,
                             out_error);
    if (!value) {
        goto failure;
    }

    if (!cos_dict_set(dict,
                      (void *)&key,
                      (void *)&value,
                      out_error)) {
        goto failure;
    }

    return true;

failure:
    if (key) {
        cos_obj_free(key);
    }
    if (value) {
        cos_obj_free(value);
    }
    return false;
}

static CosObj *
cos_handle_stream_(CosObjParser *parser,
                   const CosObjParserContext *context,
                   CosDictObj *dict_obj,
                   CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(context != NULL);
    COS_PARAMETER_ASSERT(dict_obj != NULL);
    if (COS_UNLIKELY(!parser || !context || !dict_obj)) {
        return NULL;
    }

    // ISO 32000-1:2008, Section 7.3.8 Stream Objects
    // "The stream dictionary shall be a direct object, not an indirect object."
    // "The stream dictionary shall contain the Length entry."
    // "The Length entry shall be a direct object."
    // "The value of the Length entry shall be the number of bytes from the beginning of the line
    // following the keyword stream to the last byte just before the keyword endstream."
    // "The keyword stream that follows the stream dictionary shall be followed by an end-of-line marker."
    // "The keyword endstream shall be preceded by an end-of-line marker."

    CosStreamObj *stream_obj = cos_stream_obj_create(dict_obj,
                                                     NULL);
    if (!stream_obj) {
        goto failure;
    }

    return (CosObj *)stream_obj;

failure:
    return NULL;
}

static CosObj *
cos_handle_bool_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 bool value,
                 CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(context != NULL);
    if (COS_UNLIKELY(!parser || !context)) {
        return NULL;
    }

    // Is a boolean object allowed in the current context?
    if (!(context->flags & CosObjParserFlag_BoolObj)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid boolean object"),
                            out_error);
        goto failure;
    }

    return (CosObj *)cos_bool_obj_alloc(value);

failure:
    return NULL;
}

static CosObj *
cos_handle_null_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(context != NULL);
    if (COS_UNLIKELY(!parser || !context)) {
        return NULL;
    }

    // Is a null object allowed in the current context?
    if (!(context->flags & CosObjParserFlag_NullObj)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid null object"),
                            out_error);
        goto failure;
    }

    return (CosObj *)cos_null_obj_get();

failure:
    return NULL;
}

static CosObj *
cos_handle_indirect_ref_(CosObjParser *parser,
                         const CosObjParserContext *context,
                         CosToken *obj_num_token,
                         CosToken *gen_num_token,
                         CosToken *token,
                         CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(context != NULL);
    if (COS_UNLIKELY(!parser || !context)) {
        return NULL;
    }

    // Is an indirect object reference allowed in the current context?
    if (!(context->flags & CosObjParserFlag_IndirectObjRef)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid indirect object reference"),
                            out_error);
        goto failure;
    }

    // Get the object number.
    int obj_num = 0;
    if (!cos_token_get_integer_value(obj_num_token,
                                     &obj_num)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid object number token"),
                            out_error);
        goto failure;
    }

    // Get the generation number.
    int gen_num = 0;
    if (!cos_token_get_integer_value(gen_num_token,
                                     &gen_num)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid generation number token"),
                            out_error);
        goto failure;
    }

    // Release the tokens.
    cos_tokenizer_release_token(parser->tokenizer,
                                obj_num_token);
    cos_tokenizer_release_token(parser->tokenizer,
                                gen_num_token);
    cos_tokenizer_release_token(parser->tokenizer,
                                token);

    return (CosObj *)cos_reference_obj_alloc(cos_obj_id_make((unsigned int)obj_num,
                                                             (unsigned int)gen_num),
                                             parser->doc);

failure:
    return NULL;
}

static CosObj *
cos_handle_indirect_def_(CosObjParser *parser,
                         const CosObjParserContext *context,
                         CosToken *obj_num_token,
                         CosToken *gen_num_token,
                         CosToken *token,
                         CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(context != NULL);
    if (COS_UNLIKELY(!parser || !context)) {
        return NULL;
    }

    // Is an indirect object definition allowed in the current context?
    if (!(context->flags & CosObjParserFlag_IndirectObjDef)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid indirect object definition"),
                            out_error);
        goto failure;
    }

    // ISO 19005-1:2005(E), Section 6.1.8 Indirect objects (PDF/A-1a, PDF/A-1b)
    // "The object number and generation number shall be separated by a single white-space character."
    // "The generation number and the obj keyword shall be separated by a single white-space character."
    // "The object number and endobj keyword shall each be preceded by an EOL marker."
    // "The obj and endobj keywords shall each be followed by an EOL marker.

    int obj_num = 0;
    int gen_num = 0;

    if (!cos_token_get_integer_value(obj_num_token,
                                     &obj_num)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid object number token"),
                            out_error);
        goto failure;
    }
    if (!cos_token_get_integer_value(gen_num_token,
                                     &gen_num)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid generation number token"),
                            out_error);
        goto failure;
    }

    const CosObjID obj_id = cos_obj_id_make((unsigned int)obj_num,
                                            (unsigned int)gen_num);

    // Release the tokens.
    cos_tokenizer_release_token(parser->tokenizer,
                                obj_num_token);
    cos_tokenizer_release_token(parser->tokenizer,
                                gen_num_token);
    cos_tokenizer_release_token(parser->tokenizer,
                                token);

    CosObj * const obj = cos_next_object_(parser,
                                          context,
                                          out_error);
    if (!obj) {
        goto failure;
    }

    CosToken * const endobj_token = cos_match_next_token_(parser,
                                                          CosToken_Type_EndObj,
                                                          out_error);
    if (endobj_token) {
        cos_tokenizer_release_token(parser->tokenizer,
                                    endobj_token);
    }
    else {
        printf("Expected endobj token\n");
    }

    CosIndirectObj * const indirect_obj = cos_indirect_obj_alloc(obj_id,
                                                                 obj);
    return (CosObj *)indirect_obj;

failure:
    return NULL;
}

// --------------------------------------------------------------------------

static CosObj *
cos_handle_literal_string_(CosObjParser *parser,
                           CosToken *token,
                           CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(token != NULL && token->type == CosToken_Type_Literal_String);
    if (COS_UNLIKELY(!parser || !token)) {
        return NULL;
    }

    CosData * const string_data = cos_token_move_data_value(token);
    if (!string_data) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid literal string token"),
                            error);
        goto failure;
    }

    CosStringObj * const string_obj = cos_string_obj_alloc(string_data);
    return (CosObj *)string_obj;

failure:
    return NULL;
}

static CosObj *
cos_handle_hex_string_(CosObjParser *parser,
                       CosToken *token,
                       CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(token != NULL && token->type == CosToken_Type_Hex_String);
    if (COS_UNLIKELY(!parser || !token)) {
        return NULL;
    }

    CosData * const string_data = cos_token_move_data_value(token);
    if (!string_data) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid hexadecimal string token"),
                            error);
        goto failure;
    }

    CosStringObj * const string_obj = cos_string_obj_alloc(string_data);
    return (CosObj *)string_obj;

failure:
    return NULL;
}

static CosObj *
cos_handle_name_(CosObjParser *parser,
                 CosToken *token,
                 CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(token != NULL && token->type == CosToken_Type_Name);
    if (COS_UNLIKELY(!parser || !token)) {
        return NULL;
    }

    CosString * const name = cos_token_move_string_value(token);
    if (!name) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid name token"),
                            error);
        goto failure;
    }

    return (CosObj *)cos_name_obj_alloc(name);

failure:
    return NULL;
}

static CosObj *
cos_handle_real_(CosObjParser *parser,
                 const CosToken *token,
                 CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    COS_PARAMETER_ASSERT(token != NULL && token->type == CosToken_Type_Real);
    if (COS_UNLIKELY(!parser || !token)) {
        return NULL;
    }

    double real_value = 0.0;
    if (!cos_token_value_get_real_number(token->value,
                                         &real_value)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid real token"),
                            error);
        goto failure;
    }

    CosRealObj * const real_obj = cos_real_obj_alloc(real_value);
    return (CosObj *)real_obj;

failure:
    return NULL;
}

// NOLINTEND(misc-no-recursion)

static void
cos_obj_parser_push_object_(CosObjParser *parser,
                            CosObj *object)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser || !object) {
        return;
    }

    cos_ring_buffer_push_back(parser->objects,
                              (void *)&object,
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
    cos_ring_buffer_pop_front(parser->objects, (void *)&obj);
    return obj;
}

static bool
cos_has_next_token_(CosObjParser *parser)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return false;
    }

    if (parser->next_token) {
        return true;
    }

    return cos_tokenizer_has_next_token(parser->tokenizer);
}

static const CosToken *
cos_peek_next_token_(CosObjParser *parser,
                     CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    // Does the parser have a peeked next token?
    if (parser->next_token) {
        return parser->next_token;
    }

    return cos_tokenizer_peek_next_token(parser->tokenizer,
                                         error);
}

static CosToken *
cos_get_next_token_(CosObjParser *parser,
                    CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (!parser) {
        return NULL;
    }

    // Does the parser have a peeked next token?
    if (parser->next_token) {
        CosToken * const next_token = parser->next_token;
        parser->next_token = NULL;
        return next_token;
    }

    return cos_tokenizer_get_next_token(parser->tokenizer,
                                        error);
}

static CosToken *
cos_match_next_token_(CosObjParser *parser,
                      CosToken_Type type,
                      CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (COS_UNLIKELY(!parser)) {
        return NULL;
    }

    // Check the locally-peeked next token first.
    if (parser->next_token) {
        CosToken * const next_token = parser->next_token;
        if (cos_token_get_type(next_token) != type) {
            return NULL;
        }
        parser->next_token = NULL;
        return next_token;
    }

    return cos_tokenizer_match_next_token(parser->tokenizer,
                                          type,
                                          error);
}

static bool
cos_matches_next_token_(CosObjParser *parser,
                        CosToken_Type type,
                        CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (COS_UNLIKELY(!parser)) {
        return false;
    }

    CosToken * const token = cos_match_next_token_(parser,
                                                   type,
                                                   out_error);
    if (!token) {
        return false;
    }

    cos_tokenizer_release_token(parser->tokenizer,
                                token);
    return true;
}

COS_ASSUME_NONNULL_END
