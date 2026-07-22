/*
 * Copyright (c) 2023-2024 OpenCOS.
 */

#include "CosObjParser.h"

#include "CosDoc-Private.h"
#include "common/Assert.h"
#include "common/CosCheckedArith.h"
#include "common/CosDict.h"
#include "common/CosNumber.h"
#include "objects/CosStreamObjNode-Private.h"
#include "parse/CosBaseParser.h"
#include "parse/CosParserOptions-Private.h"

#include <libcos/common/memory/CosMemory.h>
#include <libcos/xref/table/CosXrefTable.h>

#include "libcos/common/CosMacros.h"

#include <io/CosStreamReader.h>
#include <libcos/CosDoc.h>
#include <libcos/CosObjID.h>
#include <libcos/common/CosData.h>
#include <libcos/common/CosDiagnosticHandler.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosLog.h>
#include <libcos/common/CosString.h>
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

/**
 * The outcome of a single dictionary-entry or array-element parse.
 *
 * The container loops must tell apart three cases that a plain @c bool
 * conflates: reaching the closing delimiter, appending an entry and continuing,
 * and failing to parse an entry. Only the last is governed by
 * @c CosStrictGroup_ContainerEntry ; running off the end of the tokens without
 * ever reaching @c CosContainerStep_Closed is the separate
 * @c CosStrictGroup_UnterminatedContainer case, which the loop detects rather
 * than the handler.
 */
typedef enum CosContainerStep {
    /** The closing @c >> or @c ] was consumed; the container ends here. */
    CosContainerStep_Closed,

    /** An entry or element was parsed and appended; keep going. */
    CosContainerStep_Continued,

    /** The entry or element failed to parse; @c out_error holds the cause. */
    CosContainerStep_Failed,
} CosContainerStep;

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

    return cos_options_report_(&(parser->options),
                               parser->diagnostic_handler,
                               group,
                               message,
                               out_error);
}

/**
 * Applies the @c CosStrictGroup_ContainerEntry policy to a dictionary entry or
 * array element that failed to parse.
 *
 * The entry handler has already set @p out_error to the specific cause -- a
 * syntax error, or an allocation failure. The deviation is reported at the
 * group's level (silent at Off, a warning at Warn, an error at Error), but the
 * report is passed a @c NULL error so it cannot overwrite that specific cause
 * with a generic message -- which, for an allocation failure, would mislabel
 * the out-of-memory as a syntax error. At @c CosStrictLevel_Error the caller
 * then aborts with the preserved cause; at @c CosStrictLevel_Off and
 * @c CosStrictLevel_Warn the cause is discarded and the caller returns the
 * entries parsed so far.
 *
 * @return @c true if the container should recover and return what it has built;
 * @c false if the caller must abort with the cause left in @p out_error.
 */
static bool
cos_recover_container_entry_(CosObjParser *parser,
                             const char *message,
                             CosError * COS_Nullable out_error)
{
    // Report for observability, but with a NULL error so the handler's specific
    // cause in out_error survives (see the function comment).
    (void)cos_report_deviation_(&(parser->base),
                                CosStrictGroup_ContainerEntry,
                                message,
                                NULL);

    if (cos_parser_options_get_strict_level(&(parser->base.options),
                                            CosStrictGroup_ContainerEntry)
        == CosStrictLevel_Error) {
        // Abort with the handler's specific cause, left untouched in out_error.
        return false;
    }

    // Off or Warn: discard the entry's cause and recover.
    if (out_error) {
        *out_error = cos_error_none();
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

    CosObjParser * const parser = cos_calloc(1, sizeof(CosObjParser));
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
        cos_free(parser);
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

    CosObjParser * const parser = cos_calloc(1, sizeof(CosObjParser));
    if (!parser) {
        return NULL;
    }

    if (!cos_base_parser_init_with_tokenizer(&(parser->base), document, tokenizer, options)) {
        cos_free(parser);
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

    // A failed peek (undetermined) collapses to "no object" for this bool
    // predicate; callers needing the reason call cos_obj_parser_peek_object.
    CosError peek_error = CosErrorNone;
    const CosObjNode * const obj = cos_obj_parser_peek_object(parser, &peek_error);
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

static CosContainerStep
cos_handle_array_element_(CosObjParser *parser,
                          const CosObjParserContext *context,
                          CosArray *array,
                          CosError * COS_Nullable out_error);

static CosObjNode * COS_Nullable
cos_handle_dict_(CosObjParser *parser,
                 const CosObjParserContext *context,
                 CosError * COS_Nullable out_error);

static CosContainerStep
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

    CosToken * const token = cos_base_parser_get_current_token(&(parser->base), out_error);
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

    CosToken *current_token = cos_base_parser_get_current_token(&(parser->base), out_error);
    if (COS_UNLIKELY(!current_token ||
                     current_token->type != CosToken_Type_Integer)) {
        goto failure;
    }
    // We have an integer token on the stack.

    // Peek up to two more tokens to determine if this is an indirect object
    // reference or definition, or just an integer. A failed peek here is
    // absorbed as "just an integer" (see the integer_obj fallback), so its
    // reason is kept out of out_error with a throwaway error.
    CosError lookahead_error = CosErrorNone;
    CosToken * const second_token = cos_base_parser_peek_next_token(&(parser->base), 1, &lookahead_error);
    if (!second_token ||
        second_token->type != CosToken_Type_Integer) {
        goto integer_obj;
    }
    // We have two integer tokens on the stack.

    CosToken * const third_token = cos_base_parser_peek_next_token(&(parser->base), 2, &lookahead_error);
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

    CosToken * const token = cos_base_parser_get_current_token(&(parser->base), out_error);
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

    CosToken * const token = cos_base_parser_get_current_token(&(parser->base), out_error);
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

    while (true) {
        // Speculative loop guard: the array ends cleanly at the ']' that
        // cos_handle_array_element_() reports as CosContainerStep_Closed. This
        // peek only guards against running off the end of the token stream.
        //
        // A NULL peek is undetermined -- the tokenizer could not produce a
        // token -- and always propagates. A clean end of input is instead an
        // EOF token: tokens ran out before ']', an unterminated array.
        CosError peek_error = CosErrorNone;
        const CosToken * const next_token =
            cos_base_parser_peek_next_token(&(parser->base), 0, &peek_error);
        if (!next_token) {
            cos_error_propagate(out_error, peek_error);
            goto failure;
        }
        if (next_token->type == CosToken_Type_EOF) {
            if (!cos_report_deviation_(&(parser->base),
                                       CosStrictGroup_UnterminatedContainer,
                                       "Unterminated array: missing ']'",
                                       out_error)) {
                goto failure;
            }
            break;
        }

        const CosContainerStep step = cos_handle_array_element_(parser,
                                                               context,
                                                               array,
                                                               out_error);
        if (step == CosContainerStep_Closed) {
            break;
        }
        if (step == CosContainerStep_Failed) {
            if (!cos_recover_container_entry_(parser,
                                              "Malformed array element",
                                              out_error)) {
                goto failure;
            }
            break;
        }
        // CosContainerStep_Continued: parse the next element.
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

static CosContainerStep
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
        return CosContainerStep_Closed;
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

    return CosContainerStep_Continued;

failure:
    if (element) {
        cos_obj_node_release(element);
    }
    return CosContainerStep_Failed;
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

    while (true) {
        // Speculative loop guard: a dictionary ends cleanly at the '>>' that
        // cos_handle_dict_entry_() reports as CosContainerStep_Closed. This peek
        // only guards against running off the end.
        //
        // A NULL peek is undetermined -- the tokenizer could not produce a
        // token -- and always propagates. A clean end of input is instead an
        // EOF token: tokens ran out before '>>', an unterminated dictionary.
        CosError peek_error = CosErrorNone;
        const CosToken * const next_token =
            cos_base_parser_peek_next_token(&(parser->base), 0, &peek_error);
        if (!next_token) {
            cos_error_propagate(out_error, peek_error);
            goto failure;
        }
        if (next_token->type == CosToken_Type_EOF) {
            if (!cos_report_deviation_(&(parser->base),
                                       CosStrictGroup_UnterminatedContainer,
                                       "Unterminated dictionary: missing '>>'",
                                       out_error)) {
                goto failure;
            }
            break;
        }

        const CosContainerStep step = cos_handle_dict_entry_(parser,
                                                             context,
                                                             new_dict,
                                                             out_error);
        if (step == CosContainerStep_Closed) {
            break;
        }
        if (step == CosContainerStep_Failed) {
            if (!cos_recover_container_entry_(parser,
                                              "Malformed dictionary entry",
                                              out_error)) {
                goto failure;
            }
            break;
        }
        // CosContainerStep_Continued: parse the next entry.
    }

    CosDictObjNode * const dict_obj = cos_dict_obj_node_create(new_dict);
    if (!dict_obj) {
        goto failure;
    }

    // Speculative lookahead: does a stream follow this dictionary? A token that
    // is not 'stream' means a plain dictionary; a genuine stream would instead
    // desync and be caught downstream (the unconsumed 'stream' keyword).
    //
    // A NULL peek is undetermined and propagates -- releasing dict_obj, which
    // now owns new_dict -- rather than being guessed as "no stream". Any actual
    // token that is not 'stream' (including EOF) means a plain dictionary.
    CosError stream_peek_error = CosErrorNone;
    const CosToken * const lookahead_token =
        cos_base_parser_peek_next_token(&(parser->base), 0, &stream_peek_error);
    if (!lookahead_token) {
        cos_error_propagate(out_error, stream_peek_error);
        cos_obj_node_release((CosObjNode *)dict_obj);
        return NULL;
    }
    const bool is_stream = (lookahead_token->type == CosToken_Type_Stream);
    if (is_stream) {
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

static CosContainerStep
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
        return CosContainerStep_Closed;
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

    return CosContainerStep_Continued;

failure:
    if (key) {
        cos_obj_node_release(key);
    }
    if (value) {
        cos_obj_node_release(value);
    }
    return CosContainerStep_Failed;
}

// The keyword whose position bounds a stream's data during /Length recovery.
#define COS_ENDSTREAM_KEYWORD "endstream"
#define COS_ENDSTREAM_KEYWORD_LEN 9u
#define COS_ENDSTREAM_SCAN_CHUNK 8192u

/**
 * Scans a stream for the "endstream" keyword within @c [from, bound) .
 *
 * The bytes are read in overlapping chunks so that the keyword cannot be split
 * across a chunk boundary. A negative @p bound means "to the end of the stream".
 *
 * @param find_last When @c true the last match in range is returned (used with
 * an xref bound, so an "endstream" appearing inside the data cannot win); when
 * @c false the first match is returned.
 *
 * @return @c true and sets @p out_kw to the offset of the keyword if found.
 */
static bool
cos_scan_endstream_(CosStream *stream,
                    CosStreamOffset from,
                    CosStreamOffset bound,
                    bool find_last,
                    CosStreamOffset *out_kw)
{
    CosStreamOffset range_end = bound;
    if (range_end < 0) {
        if (!cos_stream_seek(stream, 0, CosStreamOffsetWhence_End, NULL)) {
            return false;
        }
        range_end = cos_stream_get_position(stream, NULL);
    }
    if (range_end < 0 ||
        (range_end - from) < (CosStreamOffset)COS_ENDSTREAM_KEYWORD_LEN) {
        return false;
    }

    const size_t overlap = COS_ENDSTREAM_KEYWORD_LEN - 1u;
    unsigned char buf[COS_ENDSTREAM_SCAN_CHUNK + (COS_ENDSTREAM_KEYWORD_LEN - 1u)];

    bool found = false;
    CosStreamOffset found_kw = 0;

    CosStreamOffset chunk_start = from;
    size_t carry = 0;                    // carried overlap bytes at buf[0..carry)
    CosStreamOffset carry_origin = from; // absolute offset of buf[0]

    while (chunk_start < range_end) {
        const CosStreamOffset remaining = range_end - chunk_start;
        size_t to_read = COS_ENDSTREAM_SCAN_CHUNK;
        if ((CosStreamOffset)to_read > remaining) {
            to_read = (size_t)remaining;
        }

        if (!cos_stream_seek(stream, chunk_start, CosStreamOffsetWhence_Set, NULL)) {
            return false;
        }
        const size_t nread = cos_stream_read(stream, buf + carry, to_read, NULL);
        if (nread == 0) {
            break;
        }
        const size_t buf_len = carry + nread;

        for (size_t j = 0; (j + COS_ENDSTREAM_KEYWORD_LEN) <= buf_len; j++) {
            if (memcmp(buf + j, COS_ENDSTREAM_KEYWORD, COS_ENDSTREAM_KEYWORD_LEN) == 0) {
                found_kw = carry_origin + (CosStreamOffset)j;
                found = true;
                if (!find_last) {
                    *out_kw = found_kw;
                    return true;
                }
            }
        }

        chunk_start += (CosStreamOffset)nread;
        if (nread < to_read) {
            break; // Reached the end of the stream.
        }

        // Carry the trailing overlap so a boundary-straddling keyword is caught.
        memmove(buf, buf + buf_len - overlap, overlap);
        carry = overlap;
        carry_origin = chunk_start - (CosStreamOffset)overlap;
    }

    if (found) {
        *out_kw = found_kw;
    }
    return found;
}

/**
 * Given the offset of an "endstream" keyword, returns the end of the stream
 * data: the byte just before the end-of-line marker preceding the keyword, per
 * ISO 32000-1 7.3.8. A missing marker is tolerated (the keyword offset is used).
 */
static CosStreamOffset
cos_stream_data_end_before_keyword_(CosStream *stream,
                                    CosStreamOffset data_start,
                                    CosStreamOffset kw)
{
    if (kw >= data_start + 2) {
        unsigned char b[2] = {0, 0};
        if (cos_stream_seek(stream, kw - 2, CosStreamOffsetWhence_Set, NULL) &&
            cos_stream_read(stream, b, 2, NULL) == 2) {
            if (b[0] == '\r' && b[1] == '\n') {
                return kw - 2;
            }
            if (b[1] == '\n' || b[1] == '\r') {
                return kw - 1;
            }
        }
    }
    else if (kw >= data_start + 1) {
        unsigned char b = 0;
        if (cos_stream_seek(stream, kw - 1, CosStreamOffsetWhence_Set, NULL) &&
            cos_stream_read(stream, &b, 1, NULL) == 1) {
            if (b == '\n' || b == '\r') {
                return kw - 1;
            }
        }
    }

    return kw;
}

/**
 * Locates the end of a stream's data by finding the "endstream" keyword.
 *
 * @param upper_bound The exclusive upper bound to search within, or a negative
 * value to search to the end of the stream.
 * @param find_last Whether the last (bounded) or first (unbounded) match wins.
 * @param out_end Set to the end offset of the stream data on success.
 */
static bool
cos_locate_endstream_(CosStream *stream,
                      CosStreamOffset data_start,
                      CosStreamOffset upper_bound,
                      bool find_last,
                      CosStreamOffset *out_end,
                      CosError * COS_Nullable out_error)
{
    CosStreamOffset kw = 0;
    if (!cos_scan_endstream_(stream, data_start, upper_bound, find_last, &kw)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_SYNTAX,
                                           "Unterminated stream: no endstream keyword"),
                            out_error);
        return false;
    }

    *out_end = cos_stream_data_end_before_keyword_(stream, data_start, kw);
    return true;
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

    CosToken * const stream_token = cos_base_parser_get_current_token(&(parser->base), out_error);
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

    // A missing, non-integer, or wrong /Length is recovered from the endstream
    // keyword rather than being fatal; see the resolution below, once data_start
    // is known. A negative value marks "no usable /Length".
    int declared_length = -1;

    CosObjNode *length_obj = NULL;
    if (cos_dict_obj_node_get_value_with_string(dict_obj,
                                           "Length",
                                           &length_obj,
                                           NULL) &&
        cos_obj_node_is_integer(length_obj)) {
        declared_length = cos_int_obj_node_get_value((CosIntObjNode *)length_obj);
    }

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

    // Resolve the stream length. A present and non-negative /Length is trusted
    // unless the Verify behaviour is selected; otherwise, and whenever /Length is
    // missing or non-integer, the extent is recovered by locating the endstream
    // keyword. The search is bounded above by the next object in the xref table
    // (so an "endstream" inside the data cannot win), falling back to the end of
    // the stream when there is no next object, and to a forward scan when the
    // table is not yet built (as while parsing an xref stream itself).
    const CosStreamLengthBehaviour length_behaviour =
        cos_parser_options_get_stream_length_behaviour(&(parser->base.options));

    const bool need_recovery =
        (declared_length < 0) ||
        (length_behaviour == CosStreamLengthBehaviour_Verify);

    size_t length;
    if (!need_recovery) {
        length = (size_t)declared_length;
    }
    else {
        const CosDoc * const doc = parser->base.doc;
        const CosXrefTable * const table =
            doc ? cos_doc_get_xref_table_(doc) : NULL;

        CosStreamOffset next_offset = 0;
        const bool has_bound =
            table &&
            cos_xref_table_find_next_offset_above(table, data_start, &next_offset);

        CosStreamOffset data_end = 0;
        bool located;
        if (has_bound) {
            // Bounded backward scan: the last endstream before the next object.
            located = cos_locate_endstream_(input_stream, data_start, next_offset,
                                            true, &data_end, out_error);
        }
        else if (table) {
            // No next object: the stream is the file's last object; bound is EOF.
            located = cos_locate_endstream_(input_stream, data_start, -1,
                                            true, &data_end, out_error);
        }
        else {
            // No table yet (xref stream): forward scan to the first endstream.
            located = cos_locate_endstream_(input_stream, data_start, -1,
                                            false, &data_end, out_error);
        }
        if (!located) {
            goto failure;
        }

        const size_t recovered_length = (size_t)(data_end - data_start);
        if (declared_length < 0) {
            if (!cos_report_deviation_(&(parser->base),
                                       CosStrictGroup_StreamLength,
                                       "Stream /Length is missing or invalid; recovered from the endstream keyword",
                                       out_error)) {
                goto failure;
            }
            length = recovered_length;
        }
        else if ((size_t)declared_length != recovered_length) {
            if (!cos_report_deviation_(&(parser->base),
                                       CosStrictGroup_StreamLength,
                                       "Stream /Length disagrees with the endstream keyword; recovered",
                                       out_error)) {
                goto failure;
            }
            length = recovered_length;
        }
        else {
            length = (size_t)declared_length;
        }
    }

    // Capture the encoded bytes as a window over the input (no copy).
    CosStream *encoded = cos_sub_stream_create(input_stream, data_start, length, false, out_error);
    if (!encoded) {
        goto failure;
    }

    // Both operands come from the file: a trusted /Length caps at INT_MAX, and a
    // recovered length is bounded by the located endstream, so the sum cannot
    // actually overflow today. The check guards against a future change to either
    // bound silently reintroducing the overflow this replaces.
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
            cos_base_parser_get_current_token(&(parser->base), out_error);
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

    // Carry the parse-time filter options onto the node, so the strictness
    // chosen at parse time governs the node's later on-demand decoding.
    const CosFilterOptions filter_options = {
        .eod_strict_level =
            cos_parser_options_get_strict_level(&(parser->base.options),
                                                CosStrictGroup_FilterEndOfData),
        .diagnostic_handler = parser->base.diagnostic_handler,
    };
    cos_stream_obj_node_set_filter_options_(stream_obj, &filter_options);

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
            cos_base_parser_get_current_token(&(parser->base), out_error);
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

    CosData *string_data = NULL;

    if (!cos_parser_context_allows_(context,
                                    CosObjParserFlag_StringObj)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid string object"),
                            error);
        goto failure;
    }

    CosToken *token = cos_base_parser_get_current_token(&(parser->base), error);
    if (COS_UNLIKELY(!token ||
                     (token->type != CosToken_Type_Literal_String &&
                      token->type != CosToken_Type_Hex_String))) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Failed to parse string object"),
                            error);
        goto failure;
    }

    string_data = cos_token_move_data_value(token);
    if (!string_data) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Failed to parse string object"),
                            error);
        goto failure;
    }

    cos_base_parser_advance(&(parser->base));

    CosStringObjNode * const string_obj = cos_string_obj_node_alloc(string_data);
    if (!string_obj) {
        // The node allocation failed, so ownership of the string data was not
        // taken; the failure path releases it.
        goto failure;
    }

    return (CosObjNode *)string_obj;

failure:
    if (string_data) {
        cos_data_free(string_data);
    }
    return NULL;
}

static CosObjNode *
cos_parse_name_(CosObjParser *parser,
                const CosObjParserContext *context,
                CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(parser != NULL);
    COS_IMPL_PARAM_CHECK(context != NULL);

    CosString *name = NULL;

    if (!cos_parser_context_allows_(context,
                                    CosObjParserFlag_NameObj)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid name object"),
                            error);
        goto failure;
    }

    CosToken * const token = cos_base_parser_get_current_token(&(parser->base), error);
    if (COS_UNLIKELY(!token ||
                     token->type != CosToken_Type_Name)) {
        goto failure;
    }

    name = cos_token_move_string_value(token);
    if (!name) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Invalid name token"),
                            error);
        goto failure;
    }

    cos_base_parser_advance(&(parser->base));

    CosNameObjNode * const nameObj = cos_name_obj_node_alloc(name);
    if (!nameObj) {
        // The node allocation failed, so ownership of the name string was not
        // taken; the failure path releases it.
        goto failure;
    }

    return (CosObjNode *)nameObj;

failure:
    if (name) {
        cos_string_free(name);
    }
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

    CosToken *token = cos_base_parser_get_current_token(&(parser->base), error);
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
