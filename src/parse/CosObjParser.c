/*
 * Copyright (c) 2023-2024 OpenCOS.
 */

#include "CosObjParser.h"

#include "common/Assert.h"
#include "common/CosDict.h"
#include "common/CosNumber.h"
#include "syntax/tokenizer/CosToken.h"
#include "syntax/tokenizer/CosTokenValue.h"
#include "syntax/tokenizer/CosTokenizer.h"

#include "libcos/common/CosMacros.h"

#include <io/CosStreamReader.h>
#include <libcos/CosDoc.h>
#include <libcos/CosObjID.h>
#include <libcos/common/CosDiagnosticHandler.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosLog.h>
#include <libcos/io/CosStream.h>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

COS_STATIC_INLINE bool
cos_parser_context_allows_(const CosObjParserContext *context,
                           CosObjParserFlags flags)
{
    return (context->flags & flags) == flags;
}

enum {
    /// The size of the parser's token buffer.
    COS_OBJ_PARSER_TOKEN_BUFFER_SIZE = 3,
};

struct CosObjParser {
    CosDoc *doc;
    CosStream *input_stream;
    CosTokenizer *tokenizer;

    CosToken * COS_Nullable token_buffer[COS_OBJ_PARSER_TOKEN_BUFFER_SIZE];
    size_t token_count;

    CosObj * COS_Nullable peeked_obj;

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
cos_parse_string_(CosObjParser *parser,
                  CosError * COS_Nullable error);

static CosObj * COS_Nullable
cos_parse_name_(CosObjParser *parser,
                CosError * COS_Nullable error)
    COS_OWNERSHIP_RETURNS;

static CosObj * COS_Nullable
cos_handle_integer_or_indirect_(CosObjParser *parser,
                                const CosObjParserContext *context,
                                CosError * COS_Nullable out_error);

static CosObj * COS_Nullable
cos_handle_integer_(CosObjParser *parser,
                    const CosObjParserContext *context,
                    CosError * COS_Nullable out_error);

static CosObj * COS_Nullable
cos_handle_real_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 CosError * COS_Nullable error);

/**
 * @brief Gets the parser's current token.
 *
 * @param parser The parser.
 *
 * @return The current token, or @c NULL if there is no current token.
 */
static CosToken * COS_Nullable
cos_current_token_(CosObjParser *parser);

/**
 * @brief Checks if there is a next token.
 *
 * @param parser The parser.
 *
 * @return @c true if there is a next token, otherwise @c false.
 */
static bool
cos_has_next_token_(CosObjParser *parser);

/**
 * @brief Peeks a next token without consuming it.
 *
 * Additional tokens are read to fill the buffer up to the specified lookahead index.
 *
 * @param parser The parser.
 * @param lookahead The lookahead index.
 *
 * @return The peeked next token, or @c NULL if there is no next token.
 */
static CosToken * COS_Nullable
cos_peek_next_token_(CosObjParser *parser,
                     unsigned int lookahead);

/**
 * @brief Advances the parser to the next token.
 *
 * This function advances the parser to the next token by shifting the token buffer,
 * releasing the current token. Additional tokens are *not* read to re-fill the buffer.
 *
 * @param parser The parser to be advanced.
 */
static void
cos_parser_advance_(CosObjParser *parser);

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

    CosTokenizer * const tokenizer = cos_tokenizer_create(input_stream);
    if (!tokenizer) {
        goto failure;
    }

    self->doc = document;
    self->input_stream = input_stream;
    self->tokenizer = tokenizer;

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
        cos_tokenizer_destroy(tokenizer);
    }
    return false;
}

void
cos_obj_parser_destroy(CosObjParser *parser)
{
    if (!parser) {
        return;
    }

    if (parser->peeked_obj) {
        cos_obj_free(COS_nonnull_cast(parser->peeked_obj));
    }

    // Release all the buffered tokens.
    for (size_t i = 0; i < parser->token_count; i++) {
        CosToken * const token = parser->token_buffer[i];
        if (token) {
            cos_token_reset(token);
            cos_token_destroy(token);
        }
    }

    cos_tokenizer_destroy(parser->tokenizer);

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
    if (parser->peeked_obj) {
        return parser->peeked_obj;
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

    parser->peeked_obj = obj;

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

    if (parser->peeked_obj) {
        CosObj * const obj = parser->peeked_obj;
        parser->peeked_obj = NULL;
        return obj;
    }

    const CosObjParserContext context = {
        .flags = CosObjParserFlag_IndirectObjDef,
    };

    return cos_next_object_(parser,
                            &context,
                            error);
}

// MARK: - Implementation

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
                   CosError * COS_Nullable out_error)
    COS_OWNERSHIP_HOLDS(3);

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
                         CosError * COS_Nullable out_error);

static CosObj * COS_Nullable
cos_handle_indirect_def_(CosObjParser *parser,
                         const CosObjParserContext *context,
                         CosError * COS_Nullable out_error);

static CosObj *
cos_next_object_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(parser != NULL);
    if (COS_UNLIKELY(!parser)) {
        return NULL;
    }

    CosToken * const token = cos_current_token_(parser);
    if (!token) {
        goto failure;
    }

    switch (token->type) {
        case CosToken_Type_Unknown:
            break;

        case CosToken_Type_Literal_String:
        case CosToken_Type_Hex_String: {
            return cos_parse_string_(parser, out_error);
        }

        case CosToken_Type_Name: {
            return cos_parse_name_(parser, out_error);
        }

        case CosToken_Type_Integer: {
            return cos_handle_integer_or_indirect_(parser,
                                                   context,
                                                   out_error);
        }
        case CosToken_Type_Real: {
            return cos_handle_real_(parser,
                                    context,
                                    out_error);
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
        case CosToken_Type_XRef: {
            printf("XRef\n");
        }
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
    return NULL;
}

static CosObj *
cos_handle_integer_or_indirect_(CosObjParser *parser,
                                const CosObjParserContext *context,
                                CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);

    CosToken *current_token = cos_current_token_(parser);
    if (COS_UNLIKELY(!current_token ||
                     current_token->type != CosToken_Type_Integer)) {
        goto failure;
    }
    // We have an integer token on the stack.

    // Peek up to two more tokens to determine if this is an indirect object reference or definition, or just an integer.
    CosToken * const second_token = cos_peek_next_token_(parser, 1);
    if (!second_token ||
        second_token->type != CosToken_Type_Integer) {
        goto integer_obj;
    }
    // We have two integer tokens on the stack.

    CosToken * const third_token = cos_peek_next_token_(parser, 2);
    if (!third_token) {
        goto integer_obj;
    }

    // If the next token is an R, then this is an indirect object reference.
    if (third_token->type == CosToken_Type_R) {
        // This is an indirect object reference.
        return cos_handle_indirect_ref_(parser,
                                        context,
                                        out_error);
    }
    // If the next token is an obj, then this is an indirect object definition.
    else if (third_token->type == CosToken_Type_Obj) {
        // This is an indirect object definition.
        return cos_handle_indirect_def_(parser,
                                        context,
                                        out_error);
    }

integer_obj:
    // This is just an integer.
    return cos_handle_integer_(parser,
                               context,
                               out_error);

failure:
    return NULL;
}

static CosObj *
cos_handle_integer_(CosObjParser *parser,
                    const CosObjParserContext *context,
                    CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);

    CosToken * const token = cos_current_token_(parser);
    if (COS_UNLIKELY(!token ||
                     token->type != CosToken_Type_Integer)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Expected an integer token"),
                            out_error);
        goto failure;
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

    cos_parser_advance_(parser);

    CosIntObj * const int_obj = cos_int_obj_alloc(int_value);
    return (CosObj *)int_obj;

failure:
    return NULL;
}

static CosObj *
cos_handle_array_(CosObjParser *parser,
                  const CosObjParserContext *context,
                  CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);

    CosArray *array = NULL;

    CosToken * const token = cos_current_token_(parser);
    if (COS_UNLIKELY(!token ||
                     token->type != CosToken_Type_ArrayStart)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Expected an array start token"),
                            out_error);
        goto failure;
    }

    cos_parser_advance_(parser);

    array = cos_array_create(sizeof(CosObj *),
                             &cos_array_obj_callbacks,
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
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);
    COS_IMPL_PARAM_CHECK(array != NULL);

    if (cos_matches_next_token_(parser,
                                CosToken_Type_ArrayEnd,
                                out_error)) {
        cos_parser_advance_(parser);
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
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);

    // Consume the dictionary start token.
    cos_parser_advance_(parser);

    CosDict *new_dict = NULL;

    new_dict = cos_dict_create(&cos_dict_obj_key_callbacks,
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
        cos_dict_destroy(new_dict);
    }
    return NULL;
}

static bool
cos_handle_dict_entry_(CosObjParser *parser,
                       const CosObjParserContext *context,
                       CosDict *dict,
                       CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);
    COS_IMPL_PARAM_CHECK(dict != NULL);

    if (cos_matches_next_token_(parser,
                                CosToken_Type_DictionaryEnd,
                                out_error)) {
        cos_parser_advance_(parser);
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
                      (void *)key,
                      (void *)value,
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
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);
    COS_IMPL_PARAM_CHECK(dict_obj != NULL);

    CosToken * const stream_token = cos_current_token_(parser);
    if (COS_UNLIKELY(!stream_token ||
                     stream_token->type != CosToken_Type_Stream)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Expected a stream token"),
                            out_error);
        goto failure;
    }

    const CosStreamOffset stream_start = (CosStreamOffset)(stream_token->offset + stream_token->length);

    // TODO: Validate the stream keyword token.
    // Consume the stream keyword.
    cos_parser_advance_(parser);

    COS_LOG_TRACE(cos_log_context_get_default(),
                  "Parsing stream object.");

    // ISO 32000-1:2008, Section 7.3.8 Stream Objects
    // "The stream dictionary shall be a direct object, not an indirect object."
    // "The stream dictionary shall contain the Length entry."
    // "The value of the Length entry shall be the number of bytes from the beginning of the line
    // following the keyword stream to the last byte just before the keyword endstream."
    // "The keyword stream that follows the stream dictionary shall be followed by an end-of-line marker."
    // "The keyword endstream shall be preceded by an end-of-line marker."

    int stream_length = -1;

    CosObj *length_obj = NULL;
    if (cos_dict_obj_get_value_with_string(dict_obj,
                                           "Length",
                                           &length_obj,
                                           NULL)) {
        if (cos_obj_is_integer(length_obj)) {
            CosIntObj * const int_obj = (CosIntObj *)length_obj;

            stream_length = cos_int_obj_get_value(int_obj);
        }
        else {
            // Error: The Length entry is not an integer.
        }
    }

    printf("Stream length: %d\n", stream_length);

    // Need to get the offset from the token.
    //    const CosStreamOffset stream_position = cos_stream_get_position(parser->input_stream,
    //                                                                    out_error);
    //    if (stream_position < 0) {
    //        goto failure;
    //    }

    // TODO: Check for overflow.
    CosStreamOffset stream_end_position = stream_start + stream_length;

    // Skip the stream data.
    if (!cos_stream_seek(parser->input_stream,
                         stream_end_position,
                         CosStreamOffsetWhence_Set,
                         out_error)) {
        goto failure;
    }

    cos_tokenizer_reset(parser->tokenizer);

    // Skip the endstream keyword.
    if (cos_matches_next_token_(parser,
                                CosToken_Type_EndStream,
                                out_error)) {
        cos_parser_advance_(parser);
    }
    else {
        // Error: Expected an endstream token.
        printf("Expected an endstream token.\n");
    }

    CosStreamObj *stream_obj = cos_stream_obj_create(dict_obj,
                                                     NULL);
    if (!stream_obj) {
        goto failure;
    }

    COS_LOG_TRACE(cos_log_context_get_default(),
                  "Stream object parsed successfully.");

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
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);

    // Is a boolean object allowed in the current context?
    if (cos_parser_context_allows_(context,
                                   CosObjParserFlag_BoolObj)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid boolean object"),
                            out_error);
        goto failure;
    }

    CosBoolObj * const bool_obj = cos_bool_obj_alloc(value);
    return (CosObj *)bool_obj;

failure:
    return NULL;
}

static CosObj *
cos_handle_null_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);

    // Is a null object allowed in the current context?
    if (cos_parser_context_allows_(context,
                                   CosObjParserFlag_NullObj)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid null object"),
                            out_error);
        goto failure;
    }

    CosNullObj * const null_obj = cos_null_obj_get();
    return (CosObj *)null_obj;

failure:
    return NULL;
}

static CosObj * COS_Nullable
cos_handle_indirect_ref_(CosObjParser *parser,
                         const CosObjParserContext *context,
                         CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);

    if (!cos_parser_context_allows_(context,
                                    CosObjParserFlag_IndirectObjRef)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid indirect object reference"),
                            out_error);
        goto failure;
    }

    COS_ASSERT(parser->token_buffer[0] != NULL && parser->token_buffer[0]->type == CosToken_Type_Integer,
               "Expected an object-number integer token");
    COS_ASSERT(parser->token_buffer[1] != NULL && parser->token_buffer[1]->type == CosToken_Type_Integer,
               "Expected a generation-number integer token");
    COS_ASSERT(parser->token_buffer[2] != NULL && parser->token_buffer[2]->type == CosToken_Type_R,
               "Expected an 'R' keyword token");

    // TODO: Validate the indirect reference tokens and whitespace.
    const CosToken * const obj_num_token = parser->token_buffer[0];
    const CosToken * const gen_num_token = parser->token_buffer[1];

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

    CosObj * const obj = (CosObj *)cos_reference_obj_alloc(cos_obj_id_make((unsigned int)obj_num,
                                                                           (unsigned int)gen_num),
                                                           parser->doc);
    if (!obj) {
        goto failure;
    }

    cos_parser_advance_(parser);
    cos_parser_advance_(parser);
    cos_parser_advance_(parser);

    return obj;

failure:
    return NULL;
}

static CosObj * COS_Nullable
cos_handle_indirect_def_(CosObjParser *parser,
                         const CosObjParserContext *context,
                         CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);

    if (!cos_parser_context_allows_(context,
                                    CosObjParserFlag_IndirectObjDef)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid indirect object definition"),
                            out_error);
        goto failure;
    }

    COS_ASSERT(parser->token_buffer[0] != NULL && parser->token_buffer[0]->type == CosToken_Type_Integer,
               "Expected an object-number integer token");
    COS_ASSERT(parser->token_buffer[1] != NULL && parser->token_buffer[1]->type == CosToken_Type_Integer,
               "Expected a generation-number integer token");
    COS_ASSERT(parser->token_buffer[2] != NULL && parser->token_buffer[2]->type == CosToken_Type_Obj,
               "Expected an 'obj' keyword token");

    CosToken * const obj_num_token = parser->token_buffer[0];
    CosToken * const gen_num_token = parser->token_buffer[1];

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

    // Consume the object header tokens.
    cos_parser_advance_(parser);
    cos_parser_advance_(parser);
    cos_parser_advance_(parser);

    const CosObjID obj_id = cos_obj_id_make((unsigned int)obj_num,
                                            (unsigned int)gen_num);

    COS_LOG_TRACE(cos_log_context_get_default(),
                  "Parsing indirect object definition: %u %u",
                  obj_id.obj_number,
                  obj_id.gen_number);

    const CosObjParserContext def_context = {
        // Allow direct and stream objects.
        .flags = (CosObjParserFlag_DirectObj |
                  CosObjParserFlag_StreamObj),
    };

    CosObj * const obj = cos_next_object_(parser,
                                          &def_context,
                                          out_error);
    if (!obj) {
        goto failure;
    }

    if (cos_matches_next_token_(parser,
                                CosToken_Type_EndObj,
                                out_error)) {
        // TODO: Validate endobj token.
        cos_parser_advance_(parser);
    }
    else {
        printf("Expected endobj token\n");
    }

    CosIndirectObj * const indirect_obj = cos_indirect_obj_alloc(obj_id,
                                                                 obj);
    if (!indirect_obj) {
        goto failure;
    }

    return (CosObj *)indirect_obj;

failure:
    return NULL;
}

// --------------------------------------------------------------------------

static CosObj *
cos_parse_string_(CosObjParser *parser,
                  CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);

    CosToken *token = cos_current_token_(parser);
    if (COS_UNLIKELY(!token ||
                     token->type != CosToken_Type_Literal_String ||
                     token->type != CosToken_Type_Hex_String)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Failed to parse string object"),
                            error);
        goto failure;
    }

    CosData * const string_data = cos_token_move_data_value(token);
    if (!string_data) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Failed to parse string object"),
                            error);
        goto failure;
    }

    cos_parser_advance_(parser);

    CosStringObj * const string_obj = cos_string_obj_alloc(string_data);
    return (CosObj *)string_obj;

failure:
    return NULL;
}

static CosObj *
cos_parse_name_(CosObjParser *parser,
                CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);

    CosToken * const token = cos_current_token_(parser);
    if (COS_UNLIKELY(!token ||
                     token->type != CosToken_Type_Name)) {
        goto failure;
    }

    CosString * const name = cos_token_move_string_value(token);
    if (!name) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid name token"),
                            error);
        goto failure;
    }

    cos_parser_advance_(parser);

    CosNameObj * const nameObj = cos_name_obj_alloc(name);
    return (CosObj *)nameObj;

failure:
    return NULL;
}

static CosObj *
cos_handle_real_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);

    if (cos_parser_context_allows_(context,
                                   CosObjParserFlag_RealObj)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid real object"),
                            error);
        goto failure;
    }

    CosToken *token = cos_current_token_(parser);
    if (COS_UNLIKELY(!token ||
                     token->type != CosToken_Type_Real)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Failed to parse real object"),
                            error);
        goto failure;
    }

    double real_value = 0.0;
    if (!cos_token_value_get_real_number(token->value,
                                         &real_value)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid real token"),
                            error);
        goto failure;
    }

    cos_parser_advance_(parser);

    CosRealObj * const real_obj = cos_real_obj_alloc(real_value);
    return (CosObj *)real_obj;

failure:
    return NULL;
}

// NOLINTEND(misc-no-recursion)

static CosToken * COS_Nullable
cos_current_token_(CosObjParser *parser)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);

    return cos_peek_next_token_(parser, 0);
}

static bool
cos_has_next_token_(CosObjParser *parser)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);

    const CosToken *token = cos_peek_next_token_(parser, 0);
    return (token != NULL);
}

static CosToken *
cos_peek_next_token_(CosObjParser *parser,
                     unsigned int lookahead)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);

    // Fill up the buffer up to the requested lookahead index.
    for (unsigned int i = 0; i <= lookahead; i++) {
        if (!parser->token_buffer[i]) {
            CosToken *token = cos_tokenizer_get_next_token(parser->tokenizer,
                                                           NULL);
            if (!token) {
                return NULL;
            }

            parser->token_buffer[i] = token;

            parser->token_count++;
        }
    }

    CosToken * const peeked_token = parser->token_buffer[lookahead];
    COS_ASSERT(peeked_token != NULL, "Expected a token");
    return peeked_token;
}

static void
cos_parser_advance_(CosObjParser *parser)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);

    if (parser->token_count == 0) {
        return;
    }

    // Release the current token.
    CosToken *current_token = parser->token_buffer[0];
    COS_ASSERT(current_token != NULL, "Expected a token");
    cos_token_reset(current_token);
    cos_token_destroy(current_token);

    const size_t token_buffer_size = COS_ARRAY_SIZE(parser->token_buffer);

    // "Pop" the first token by moving the rest of the buffer left with memmove.
    memmove(&parser->token_buffer[0],
            &parser->token_buffer[1],
            sizeof(CosToken *) * (token_buffer_size - 1));

    // Zero out the last token after the shift.
    parser->token_buffer[token_buffer_size - 1] = NULL;

    // Decrement the token count.
    parser->token_count--;
}

static bool
cos_matches_next_token_(CosObjParser *parser,
                        CosToken_Type type,
                        CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);

    CosToken * const token = cos_peek_next_token_(parser, 0);
    if (!token) {
        return false;
    }

    return (token->type == type);
}

COS_ASSUME_NONNULL_END
