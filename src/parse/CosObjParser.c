/*
 * Copyright (c) 2023-2024 OpenCOS.
 */

#include "CosObjParser.h"

#include "common/Assert.h"
#include "common/CosCheckedArith.h"
#include "common/CosDict.h"
#include "common/CosNumber.h"
#include "parse/CosBaseParser.h"
#include "parse/CosParserOptions-Private.h"

#include "libcos/common/CosMacros.h"

#include <io/CosStreamReader.h>
#include <libcos/CosDoc.h>
#include <libcos/CosObjID.h>
#include <libcos/common/CosDiagnosticHandler.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosLog.h>
#include <libcos/io/CosStream.h>
#include <libcos/io/CosSubStream.h>
#include <libcos/objects/CosArrayObjNode.h>
#include <libcos/objects/CosBoolObjNode.h>
#include <libcos/objects/CosDictObjNode.h>
#include <libcos/objects/CosIndirectObjNode.h>
#include <libcos/objects/CosIntObjNode.h>
#include <libcos/objects/CosNameObjNode.h>
#include <libcos/objects/CosNullObjNode.h>
#include <libcos/objects/CosObjNode.h>
#include <libcos/objects/CosRealObjNode.h>
#include <libcos/objects/CosReferenceObjNode.h>
#include <libcos/objects/CosStreamObjNode.h>
#include <libcos/objects/CosStringObjNode.h>
#include <libcos/syntax/tokenizer/CosToken.h>
#include <libcos/syntax/tokenizer/CosTokenValue.h>
#include <libcos/syntax/tokenizer/CosTokenizer.h>

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

typedef struct CosObjParserContext {
    CosObjParserFlags flags;
} CosObjParserContext;

COS_STATIC_INLINE bool
cos_parser_context_allows_(const CosObjParserContext *context,
                           CosObjParserFlags flags)
{
    return (context->flags & flags) == flags;
}

/**
 * The default top-level context: a bare direct object or an indirect object
 * definition. Notably excludes indirect references, which are legal only as a
 * direct-object value, never on their own.
 */
#define COS_OBJ_PARSER_DEFAULT_TOP_LEVEL_FLAGS \
    (CosObjParserFlag_DirectObj | CosObjParserFlag_IndirectObjDef)

struct CosObjParser {
    CosBaseParser base;

    CosObjNode * COS_Nullable peeked_node;

    CosObjParserFlags top_level_flags;
};

static bool
cos_obj_parser_init_(CosObjParser *self,
                     CosDoc *document,
                     CosStream *input_stream,
                     const CosParserOptions * COS_Nullable options);

/**
 * Reports a deviation from the specification under a strict-mode group.
 *
 * @param parser The parser whose options and diagnostic handler to use.
 * @param group The group governing this deviation.
 * @param message The message to report. Must be a string literal or otherwise
 * outlive the call, as diagnostics do not copy it.
 * @param out_error Set to a @c COS_ERROR_SYNTAX error if the group escalates.
 *
 * @return @c true if parsing may continue, @c false if the group's level is
 * @c CosStrictLevel_Error and the caller must abort.
 */
static bool
cos_report_deviation_(CosBaseParser *parser,
                      CosStrictGroup group,
                      const char *message,
                      CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(4);

static CosObjNode * COS_Nullable
cos_next_object_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 CosError * COS_Nullable out_error)
    COS_OWNERSHIP_RETURNS;

static CosObjNode * COS_Nullable
cos_parse_string_(CosObjParser *parser,
                  const CosObjParserContext *context,
                  CosError * COS_Nullable error);

static CosObjNode * COS_Nullable
cos_parse_name_(CosObjParser *parser,
                const CosObjParserContext *context,
                CosError * COS_Nullable error)
    COS_OWNERSHIP_RETURNS;

static CosObjNode * COS_Nullable
cos_handle_integer_or_indirect_(CosObjParser *parser,
                                const CosObjParserContext *context,
                                CosError * COS_Nullable out_error);

static CosObjNode * COS_Nullable
cos_handle_integer_(CosObjParser *parser,
                    const CosObjParserContext *context,
                    CosError * COS_Nullable out_error);

static CosObjNode * COS_Nullable
cos_handle_real_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 CosError * COS_Nullable error);

static bool
cos_report_deviation_(CosBaseParser *parser,
                      CosStrictGroup group,
                      const char *message,
                      CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(message != NULL);
    if (!parser || !message) {
        return true;
    }

    const CosStrictLevel level =
        cos_parser_options_get_strict_level(&(parser->options), group);
    if (level == CosStrictLevel_Off) {
        return true;
    }

    if (parser->diagnostic_handler) {
        cos_diagnose(parser->diagnostic_handler,
                     (level == CosStrictLevel_Error) ? CosDiagnosticLevel_Error
                                                     : CosDiagnosticLevel_Warning,
                     message);
    }

    if (level == CosStrictLevel_Error) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_SYNTAX, message),
                            out_error);
        return false;
    }

    return true;
}

CosObjParser *
cos_obj_parser_create(CosDoc *document,
                      CosStream *input_stream,
                      const CosParserOptions * COS_Nullable options)
{
    COS_API_PARAM_CHECK(document != NULL);
    COS_API_PARAM_CHECK(input_stream != NULL);
    if (!input_stream) {
        return NULL;
    }

    CosObjParser * const parser = calloc(1, sizeof(CosObjParser));
    if (!parser) {
        goto failure;
    }

    if (!cos_obj_parser_init_(parser,
                              document,
                              input_stream,
                              options)) {
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
                     CosStream *input_stream,
                     const CosParserOptions * COS_Nullable options)
{
    COS_IMPL_PARAM_CHECK(self != NULL);
    COS_IMPL_PARAM_CHECK(document != NULL);
    COS_IMPL_PARAM_CHECK(input_stream != NULL);
    if (!self || !document || !input_stream) {
        return false;
    }

    self->top_level_flags = COS_OBJ_PARSER_DEFAULT_TOP_LEVEL_FLAGS;

    return cos_base_parser_init(&(self->base),
                                document,
                                input_stream,
                                options);
}

CosObjParser *
cos_obj_parser_create_with_tokenizer(CosDoc *document,
                                     CosTokenizer *tokenizer,
                                     const CosParserOptions * COS_Nullable options)
{
    COS_API_PARAM_CHECK(document != NULL);
    COS_API_PARAM_CHECK(tokenizer != NULL);
    if (!document || !tokenizer) {
        return NULL;
    }

    CosObjParser * const parser = calloc(1, sizeof(CosObjParser));
    if (!parser) {
        return NULL;
    }

    if (!cos_base_parser_init_with_tokenizer(&(parser->base), document, tokenizer, options)) {
        free(parser);
        return NULL;
    }

    parser->top_level_flags = COS_OBJ_PARSER_DEFAULT_TOP_LEVEL_FLAGS;

    return parser;
}

void
cos_obj_parser_destroy(CosObjParser *parser)
{
    if (!parser) {
        return;
    }

    if (parser->peeked_node) {
        cos_obj_node_release(COS_nonnull_cast(parser->peeked_node));
    }

    cos_base_parser_destroy(&(parser->base));
}

void
cos_obj_parser_flush_tokens_(CosObjParser *parser)
{
    if (!parser) {
        return;
    }

    // A peeked node was parsed at the previous stream position; a flush follows
    // a seek, so it is stale. Drop it along with the buffered tokens.
    if (parser->peeked_node) {
        cos_obj_node_release(COS_nonnull_cast(parser->peeked_node));
        parser->peeked_node = NULL;
    }

    CosBaseParser * const base = &parser->base;
    for (size_t i = 0; i < base->token_count; i++) {
        cos_token_reset(&(base->token_buffer[i]));
    }
    base->token_count = 0;
}

void
cos_obj_parser_set_top_level_flags_(CosObjParser *parser,
                                    CosObjParserFlags flags)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    if (!parser) {
        return;
    }

    parser->top_level_flags = flags;
}

bool
cos_obj_parser_has_next_object(CosObjParser *parser)
{
    COS_API_PARAM_CHECK(parser != NULL);
    if (!parser) {
        return false;
    }

    const CosObjNode * const obj = cos_obj_parser_peek_object(parser, NULL);
    return (obj != NULL);
}

CosObjNode *
cos_obj_parser_peek_object(CosObjParser *parser,
                           CosError * COS_Nullable error)
{
    COS_API_PARAM_CHECK(parser != NULL);
    if (!parser) {
        return NULL;
    }

    // If there are objects in the queue, return the first one.
    if (parser->peeked_node) {
        return parser->peeked_node;
    }

    const CosObjParserContext context = {
        .flags = parser->top_level_flags,
    };

    // Otherwise, parse the next obj and push it to the queue.
    CosError error_ = cos_error_none();
    CosObjNode * const obj = cos_next_object_(parser,
                                          &context,
                                          &error_);
    if (!obj) {
        COS_ERROR_PROPAGATE(error_, error);
        return NULL;
    }

    parser->peeked_node = obj;

    return obj;
}

CosObjNode *
cos_obj_parser_next_object(CosObjParser *parser,
                           CosError * COS_Nullable error)
{
    COS_API_PARAM_CHECK(parser != NULL);
    if (!parser) {
        return NULL;
    }

    if (parser->peeked_node) {
        CosObjNode * const obj = parser->peeked_node;
        parser->peeked_node = NULL;
        return obj;
    }

    const CosObjParserContext context = {
        .flags = parser->top_level_flags,
    };

    return cos_next_object_(parser,
                            &context,
                            error);
}

// MARK: - Implementation

// NOLINTBEGIN(misc-no-recursion)

static CosObjNode * COS_Nullable
cos_handle_array_(CosObjParser *parser,
                  const CosObjParserContext *context,
                  CosError * COS_Nullable out_error);

static bool
cos_handle_array_element_(CosObjParser *parser,
                          const CosObjParserContext *context,
                          CosArray *array,
                          CosError * COS_Nullable out_error);

static CosObjNode * COS_Nullable
cos_handle_dict_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 CosError * COS_Nullable out_error);

static bool
cos_handle_dict_entry_(CosObjParser *parser,
                       const CosObjParserContext *context,
                       CosDict *dict,
                       CosError * COS_Nullable out_error);

static CosObjNode * COS_Nullable
cos_handle_stream_(CosObjParser *parser,
                   const CosObjParserContext *context,
                   CosDictObjNode *dict_obj,
                   CosError * COS_Nullable out_error)
    COS_OWNERSHIP_TAKES(3);

static CosObjNode * COS_Nullable
cos_handle_bool_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 bool value,
                 CosError * COS_Nullable out_error);

static CosObjNode * COS_Nullable
cos_handle_null_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 CosError * COS_Nullable out_error);

static CosObjNode * COS_Nullable
cos_handle_indirect_ref_(CosObjParser *parser,
                         const CosObjParserContext *context,
                         CosError * COS_Nullable out_error);

static CosObjNode * COS_Nullable
cos_handle_indirect_def_(CosObjParser *parser,
                         const CosObjParserContext *context,
                         CosError * COS_Nullable out_error);

static CosObjNode *
cos_next_object_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    if (COS_UNLIKELY(!parser)) {
        return NULL;
    }

    CosToken * const token = cos_base_parser_get_current_token(&(parser->base));
    if (!token) {
        goto failure;
    }

    switch (token->type) {
        case CosToken_Type_Unknown:
            break;

        case CosToken_Type_Literal_String:
        case CosToken_Type_Hex_String: {
            return cos_parse_string_(parser, context, out_error);
        }

        case CosToken_Type_Name: {
            return cos_parse_name_(parser, context, out_error);
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
    return NULL;
}

static CosObjNode *
cos_handle_integer_or_indirect_(CosObjParser *parser,
                                const CosObjParserContext *context,
                                CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);

    CosToken *current_token = cos_base_parser_get_current_token(&(parser->base));
    if (COS_UNLIKELY(!current_token ||
                     current_token->type != CosToken_Type_Integer)) {
        goto failure;
    }
    // We have an integer token on the stack.

    // Peek up to two more tokens to determine if this is an indirect object reference or definition, or just an integer.
    CosToken * const second_token = cos_base_parser_peek_next_token(&(parser->base), 1);
    if (!second_token ||
        second_token->type != CosToken_Type_Integer) {
        goto integer_obj;
    }
    // We have two integer tokens on the stack.

    CosToken * const third_token = cos_base_parser_peek_next_token(&(parser->base), 2);
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

static CosObjNode *
cos_handle_integer_(CosObjParser *parser,
                    const CosObjParserContext *context,
                    CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);

    if (!cos_parser_context_allows_(context,
                                    CosObjParserFlag_IntObj)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid integer object"),
                            out_error);
        goto failure;
    }

    CosToken * const token = cos_base_parser_get_current_token(&(parser->base));
    if (COS_UNLIKELY(!token ||
                     token->type != CosToken_Type_Integer)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Expected an integer token"),
                            out_error);
        goto failure;
    }

    // Get the integer value of the token.
    int int_value = 0;
    if (!cos_token_value_get_integer_number(&token->value,
                                            &int_value)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid integer token"),
                            out_error);
        goto failure;
    }

    cos_base_parser_advance(&(parser->base));

    CosIntObjNode * const int_obj = cos_int_obj_node_alloc(int_value);
    return (CosObjNode *)int_obj;

failure:
    return NULL;
}

static CosObjNode *
cos_handle_array_(CosObjParser *parser,
                  const CosObjParserContext *context,
                  CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);

    CosArray *array = NULL;

    if (!cos_parser_context_allows_(context,
                                    CosObjParserFlag_ArrayObj)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid array object"),
                            out_error);
        goto failure;
    }

    CosToken * const token = cos_base_parser_get_current_token(&(parser->base));
    if (COS_UNLIKELY(!token ||
                     token->type != CosToken_Type_ArrayStart)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Expected an array start token"),
                            out_error);
        goto failure;
    }

    cos_base_parser_advance(&(parser->base));

    array = cos_array_create(sizeof(CosObjNode *),
                             &cos_array_obj_node_callbacks,
                             0);
    if (!array) {
        goto failure;
    }

    bool element_success = false;
    while (cos_base_parser_has_next_token(&(parser->base))) {
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

    CosArrayObjNode * const array_obj = cos_array_obj_node_alloc(array);
    if (!array_obj) {
        goto failure;
    }

    return (CosObjNode *)array_obj;

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

    if (cos_base_parser_matches_next_token(&(parser->base),
                                           CosToken_Type_ArrayEnd,
                                           out_error)) {
        cos_base_parser_advance(&(parser->base));
        return false;
    }

    CosObjNode *element = NULL;

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
        cos_obj_node_release(element);
    }
    return false;
}

static CosObjNode *
cos_handle_dict_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);

    CosDict *new_dict = NULL;

    if (!cos_parser_context_allows_(context,
                                    CosObjParserFlag_DictObj)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid dictionary object"),
                            out_error);
        goto failure;
    }

    // Consume the dictionary start token.
    cos_base_parser_advance(&(parser->base));

    new_dict = cos_dict_create(&cos_dict_obj_node_key_callbacks,
                               &cos_dict_obj_node_value_callbacks,
                               0);
    if (!new_dict) {
        goto failure;
    }

    bool entry_success = false;
    while (cos_base_parser_has_next_token(&(parser->base))) {
        entry_success = cos_handle_dict_entry_(parser,
                                               context,
                                               new_dict,
                                               out_error);
        if (!entry_success) {
            // Or, skip entry and continue parsing?
            break;
        }
    }

    CosDictObjNode * const dict_obj = cos_dict_obj_node_create(new_dict);
    if (!dict_obj) {
        goto failure;
    }

    // Check if the next token denotes the beginning of a stream object's data.
    if (cos_base_parser_matches_next_token(&(parser->base),
                                           CosToken_Type_Stream,
                                           out_error)) {
        return cos_handle_stream_(parser,
                                  context,
                                  dict_obj,
                                  out_error);
    }

    return (CosObjNode *)dict_obj;

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

    if (cos_base_parser_matches_next_token(&(parser->base),
                                           CosToken_Type_DictionaryEnd,
                                           out_error)) {
        cos_base_parser_advance(&(parser->base));
        return false;
    }

    CosObjNode *key = NULL;
    CosObjNode *value = NULL;

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

    /*
     * key_context permits only name objects, so a malformed dictionary such as
     * "<< 1 2 >>" already fails in cos_next_object_() above. This check is
     * belt-and-suspenders on the unchecked cast to CosNameObjNode in the
     * CosDictObjNode key callbacks: a non-name key would read past the end of a
     * smaller node, so reject it here too, where the type is still known.
     */
    if (cos_obj_node_get_type(key) != CosObjNodeType_Name) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_SYNTAX,
                                           "Dictionary key is not a name object"),
                            out_error);
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
        cos_obj_node_release(key);
    }
    if (value) {
        cos_obj_node_release(value);
    }
    return false;
}

static CosObjNode *
cos_handle_stream_(CosObjParser *parser,
                   const CosObjParserContext *context,
                   CosDictObjNode *dict_obj,
                   CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);
    COS_IMPL_PARAM_CHECK(dict_obj != NULL);

    if (!cos_parser_context_allows_(context,
                                    CosObjParserFlag_StreamObj)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid stream object"),
                            out_error);
        goto failure;
    }

    CosToken * const stream_token = cos_base_parser_get_current_token(&(parser->base));
    if (COS_UNLIKELY(!stream_token ||
                     stream_token->type != CosToken_Type_Stream)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Expected a stream token"),
                            out_error);
        goto failure;
    }

    const CosStreamOffset stream_start = (CosStreamOffset)(stream_token->offset + stream_token->length);

    // The keyword's type was checked above; the end-of-line marker that must
    // follow it is checked once the data offset has been sniffed below.
    // Consume the stream keyword.
    cos_base_parser_advance(&(parser->base));

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

    CosObjNode *length_obj = NULL;
    if (cos_dict_obj_node_get_value_with_string(dict_obj,
                                           "Length",
                                           &length_obj,
                                           NULL) &&
        cos_obj_node_is_integer(length_obj)) {
        stream_length = cos_int_obj_node_get_value((CosIntObjNode *)length_obj);
    }

    if (stream_length < 0) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_SYNTAX,
                                           "Stream is missing a valid /Length"),
                            out_error);
        goto failure;
    }

    const size_t length = (size_t)stream_length;

    // The tokenizer's stream token stops at the "stream" keyword; the data begins after the
    // end-of-line marker (CRLF or LF) that follows it. Only seekable inputs are supported.
    // The object parser borrows the tokenizer, so obtain the input stream from it.
    CosStream * const input_stream = cos_tokenizer_get_input_stream(parser->base.tokenizer);
    if (!cos_stream_can_seek(input_stream)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_NOT_IMPLEMENTED,
                                           "Parsing streams from a non-seekable input is not supported"),
                            out_error);
        goto failure;
    }

    if (!cos_stream_seek(input_stream, stream_start, CosStreamOffsetWhence_Set, out_error)) {
        goto failure;
    }

    unsigned char eol[2] = {0, 0};
    const size_t eol_read = cos_stream_read(input_stream, eol, sizeof(eol), NULL);
    CosStreamOffset data_start = stream_start;
    if (eol_read >= 2 && eol[0] == '\r' && eol[1] == '\n') {
        data_start = stream_start + 2;
    }
    else if (eol_read >= 1 && (eol[0] == '\n' || eol[0] == '\r')) {
        data_start = stream_start + 1;
    }

    // The data offset above stays deliberately lenient. These only report what
    // it silently accepted: ISO 32000-1 allows LF or CRLF here, nothing else.
    if (data_start == stream_start) {
        if (!cos_report_deviation_(&(parser->base),
                                   CosStrictGroup_EolMarkers,
                                   "Missing end-of-line marker after the stream keyword",
                                   out_error)) {
            goto failure;
        }
    }
    else if (eol[0] == '\r' && data_start == (stream_start + 1)) {
        if (!cos_report_deviation_(&(parser->base),
                                   CosStrictGroup_EolMarkers,
                                   "Bare carriage return after the stream keyword",
                                   out_error)) {
            goto failure;
        }
    }

    // Capture the encoded bytes as a window over the input (no copy).
    CosStream *encoded = cos_sub_stream_create(input_stream, data_start, length, false, out_error);
    if (!encoded) {
        goto failure;
    }

    // Both operands come from the file. The sum cannot actually overflow today,
    // because /Length is parsed into an int and rejected when negative, so it
    // caps at INT_MAX; the check is here so that widening that parse later
    // cannot silently reintroduce the overflow this replaces.
    CosStreamOffset stream_end_position;
    if (cos_ckd_add_off_(&stream_end_position, data_start, (CosStreamOffset)length)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_SYNTAX,
                                           "Stream length overflows the stream offset range"),
                            out_error);
        cos_stream_close(encoded);
        goto failure;
    }

    // Seek past the stream data to the endstream keyword.
    if (!cos_stream_seek(input_stream,
                         stream_end_position,
                         CosStreamOffsetWhence_Set,
                         out_error)) {
        cos_stream_close(encoded);
        goto failure;
    }

    cos_tokenizer_reset(parser->base.tokenizer);

    // Skip the endstream keyword. The data is length-delimited, so a missing endstream is
    // tolerated rather than fatal.
    if (cos_base_parser_matches_next_token(&(parser->base),
                                           CosToken_Type_EndStream,
                                           out_error)) {
        // "The keyword endstream shall be preceded by an end-of-line marker."
        const CosToken * const endstream_token =
            cos_base_parser_get_current_token(&(parser->base));
        if (endstream_token &&
            !cos_token_whitespace_is_eol(&(endstream_token->leading_whitespace))) {
            if (!cos_report_deviation_(&(parser->base),
                                       CosStrictGroup_EolMarkers,
                                       "Missing end-of-line marker before the endstream keyword",
                                       out_error)) {
                cos_stream_close(encoded);
                goto failure;
            }
        }

        cos_base_parser_advance(&(parser->base));
    }
    else {
        if (!cos_report_deviation_(&(parser->base),
                                   CosStrictGroup_RequiredKeywords,
                                   "Expected an endstream keyword",
                                   out_error)) {
            cos_stream_close(encoded);
            goto failure;
        }
    }

    CosStreamObjNode * const stream_obj = cos_stream_obj_node_create(dict_obj, encoded);
    if (!stream_obj) {
        // cos_stream_obj_node_create closed `encoded` but not `dict_obj`.
        goto failure;
    }

    COS_LOG_TRACE(cos_log_context_get_default(),
                  "Stream object parsed successfully.");

    return (CosObjNode *)stream_obj;

failure:
    // Ownership of the dictionary transferred in with the call; only a
    // successful cos_stream_obj_node_create() takes it over.
    cos_obj_node_release((CosObjNode *)dict_obj);

    return NULL;
}

static CosObjNode *
cos_handle_bool_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 bool value,
                 CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);

    // Is a boolean object allowed in the current context?
    if (!cos_parser_context_allows_(context,
                                    CosObjParserFlag_BoolObj)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid boolean object"),
                            out_error);
        goto failure;
    }

    cos_base_parser_advance(&(parser->base));

    CosBoolObjNode * const bool_obj = cos_bool_obj_node_alloc(value);
    return (CosObjNode *)bool_obj;

failure:
    return NULL;
}

static CosObjNode *
cos_handle_null_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);

    // Is a null object allowed in the current context?
    if (!cos_parser_context_allows_(context,
                                    CosObjParserFlag_NullObj)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid null object"),
                            out_error);
        goto failure;
    }

    cos_base_parser_advance(&(parser->base));

    CosNullObjNode * const null_obj = cos_null_obj_node_get();
    return (CosObjNode *)null_obj;

failure:
    return NULL;
}

/**
 * Checks the whitespace separating the three tokens of an "N G R" or
 * "N G obj" header.
 *
 * ISO 19005-1:2005(E), Section 6.1.8: the object number and generation number
 * shall be separated by a single white-space character, as shall the
 * generation number and the keyword that follows it.
 *
 * The whitespace predicates are used rather than offset arithmetic because a
 * comment between two tokens is excluded from the character count.
 *
 * @param parser The parser holding the three header tokens.
 * @param out_error Set if the deviation is escalated to an error.
 *
 * @return @c true if parsing may continue, otherwise @c false.
 */
static bool
cos_check_obj_header_spacing_(CosBaseParser *parser,
                              CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(2);

static bool
cos_check_obj_header_spacing_(CosBaseParser *parser,
                              CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    if (!parser) {
        return true;
    }

    if (!cos_token_whitespace_is_single_space(&(parser->token_buffer[1].leading_whitespace))) {
        if (!cos_report_deviation_(parser,
                                   CosStrictGroup_ObjHeaderSpacing,
                                   "Expected a single space between the object and generation numbers",
                                   out_error)) {
            return false;
        }
    }

    if (!cos_token_whitespace_is_single_space(&(parser->token_buffer[2].leading_whitespace))) {
        if (!cos_report_deviation_(parser,
                                   CosStrictGroup_ObjHeaderSpacing,
                                   "Expected a single space before the object header keyword",
                                   out_error)) {
            return false;
        }
    }

    return true;
}

/**
 * Rejects a negative object or generation number.
 *
 * Deliberately not governed by a strict-mode group. Both numbers are cast to
 * unsigned when the object ID is built, so a negative value would silently
 * become a very large one; that is a correctness problem rather than a
 * conformance preference, and must not be suppressible.
 *
 * @param obj_num The parsed object number.
 * @param gen_num The parsed generation number.
 * @param out_error Set if either number is negative.
 *
 * @return @c true if both numbers are usable, otherwise @c false.
 */
static bool
cos_check_obj_id_numbers_(int obj_num,
                          int gen_num,
                          CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

static bool
cos_check_obj_id_numbers_(int obj_num,
                          int gen_num,
                          CosError * COS_Nullable out_error)
{
    if (obj_num < 0 || gen_num < 0) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_SYNTAX,
                                           "Negative object or generation number"),
                            out_error);
        return false;
    }

    return true;
}

static CosObjNode * COS_Nullable
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

    COS_ASSERT(parser->base.token_buffer[0].type == CosToken_Type_Integer,
               "Expected an object-number integer token");
    COS_ASSERT(parser->base.token_buffer[1].type == CosToken_Type_Integer,
               "Expected a generation-number integer token");
    COS_ASSERT(parser->base.token_buffer[2].type == CosToken_Type_R,
               "Expected an 'R' keyword token");

    if (!cos_check_obj_header_spacing_(&(parser->base), out_error)) {
        goto failure;
    }

    const CosToken * const obj_num_token = &parser->base.token_buffer[0];
    const CosToken * const gen_num_token = &parser->base.token_buffer[1];

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

    if (!cos_check_obj_id_numbers_(obj_num, gen_num, out_error)) {
        goto failure;
    }

    CosObjNode * const obj = (CosObjNode *)cos_reference_obj_node_alloc(cos_obj_id_make((unsigned int)obj_num,
                                                                           (unsigned int)gen_num),
                                                           parser->base.doc);
    if (!obj) {
        goto failure;
    }

    cos_base_parser_advance(&(parser->base));
    cos_base_parser_advance(&(parser->base));
    cos_base_parser_advance(&(parser->base));

    return obj;

failure:
    return NULL;
}

static CosObjNode * COS_Nullable
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

    COS_ASSERT(parser->base.token_buffer[0].type == CosToken_Type_Integer,
               "Expected an object-number integer token");
    COS_ASSERT(parser->base.token_buffer[1].type == CosToken_Type_Integer,
               "Expected a generation-number integer token");
    COS_ASSERT(parser->base.token_buffer[2].type == CosToken_Type_Obj,
               "Expected an 'obj' keyword token");

    CosToken * const obj_num_token = &parser->base.token_buffer[0];
    CosToken * const gen_num_token = &parser->base.token_buffer[1];

    // ISO 19005-1:2005(E), Section 6.1.8 Indirect objects (PDF/A-1a, PDF/A-1b)
    // "The object number and generation number shall be separated by a single white-space character."
    // "The generation number and the obj keyword shall be separated by a single white-space character."
    // "The object number and endobj keyword shall each be preceded by an EOL marker."
    // "The obj and endobj keywords shall each be followed by an EOL marker.

    if (!cos_check_obj_header_spacing_(&(parser->base), out_error)) {
        goto failure;
    }

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

    if (!cos_check_obj_id_numbers_(obj_num, gen_num, out_error)) {
        goto failure;
    }

    // Consume the object header tokens.
    cos_base_parser_advance(&(parser->base));
    cos_base_parser_advance(&(parser->base));
    cos_base_parser_advance(&(parser->base));

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

    CosObjNode * const obj = cos_next_object_(parser,
                                          &def_context,
                                          out_error);
    if (!obj) {
        goto failure;
    }

    if (cos_base_parser_matches_next_token(&(parser->base),
                                           CosToken_Type_EndObj,
                                           out_error)) {
        // "The object number and endobj keyword shall each be preceded by an
        // EOL marker."
        const CosToken * const endobj_token =
            cos_base_parser_get_current_token(&(parser->base));
        if (endobj_token &&
            !cos_token_whitespace_is_eol(&(endobj_token->leading_whitespace))) {
            if (!cos_report_deviation_(&(parser->base),
                                       CosStrictGroup_EolMarkers,
                                       "Missing end-of-line marker before the endobj keyword",
                                       out_error)) {
                goto obj_failure;
            }
        }

        cos_base_parser_advance(&(parser->base));
    }
    else {
        // A missing endobj is tolerated by default: the object was already
        // parsed. Reported so that callers can choose to reject it.
        if (!cos_report_deviation_(&(parser->base),
                                   CosStrictGroup_RequiredKeywords,
                                   "Expected an endobj keyword",
                                   out_error)) {
            goto obj_failure;
        }
    }

    CosIndirectObjNode * const indirect_obj = cos_indirect_obj_node_alloc(obj_id,
                                                                 obj);
    if (!indirect_obj) {
        goto obj_failure;
    }

    COS_LOG_TRACE(cos_log_context_get_default(),
                  "Parsed indirect object definition: %u %u",
                  obj_id.obj_number,
                  obj_id.gen_number);

    return (CosObjNode *)indirect_obj;

obj_failure:
    // Reached only after cos_next_object_ succeeded, so the object is owned
    // here until cos_indirect_obj_node_alloc() takes it over.
    cos_obj_node_release(obj);

failure:
    return NULL;
}

// --------------------------------------------------------------------------

static CosObjNode *
cos_parse_string_(CosObjParser *parser,
                  const CosObjParserContext *context,
                  CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);

    if (!cos_parser_context_allows_(context,
                                    CosObjParserFlag_StringObj)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid string object"),
                            error);
        goto failure;
    }

    CosToken *token = cos_base_parser_get_current_token(&(parser->base));
    if (COS_UNLIKELY(!token ||
                     (token->type != CosToken_Type_Literal_String &&
                      token->type != CosToken_Type_Hex_String))) {
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

    cos_base_parser_advance(&(parser->base));

    CosStringObjNode * const string_obj = cos_string_obj_node_alloc(string_data);
    return (CosObjNode *)string_obj;

failure:
    return NULL;
}

static CosObjNode *
cos_parse_name_(CosObjParser *parser,
                const CosObjParserContext *context,
                CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);

    if (!cos_parser_context_allows_(context,
                                    CosObjParserFlag_NameObj)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid name object"),
                            error);
        goto failure;
    }

    CosToken * const token = cos_base_parser_get_current_token(&(parser->base));
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

    cos_base_parser_advance(&(parser->base));

    CosNameObjNode * const nameObj = cos_name_obj_node_alloc(name);
    return (CosObjNode *)nameObj;

failure:
    return NULL;
}

static CosObjNode *
cos_handle_real_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);

    if (!cos_parser_context_allows_(context,
                                    CosObjParserFlag_RealObj)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid real object"),
                            error);
        goto failure;
    }

    CosToken *token = cos_base_parser_get_current_token(&(parser->base));
    if (COS_UNLIKELY(!token ||
                     token->type != CosToken_Type_Real)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Failed to parse real object"),
                            error);
        goto failure;
    }

    double real_value = 0.0;
    if (!cos_token_value_get_real_number(&token->value,
                                         &real_value)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid real token"),
                            error);
        goto failure;
    }

    cos_base_parser_advance(&(parser->base));

    CosRealObjNode * const real_obj = cos_real_obj_node_alloc(real_value);
    return (CosObjNode *)real_obj;

failure:
    return NULL;
}

// NOLINTEND(misc-no-recursion)

COS_ASSUME_NONNULL_END
