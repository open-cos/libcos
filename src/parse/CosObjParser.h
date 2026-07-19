//
// Created by david on 29/06/23.
//

#ifndef LIBCOS_COS_OBJ_PARSER_H
#define LIBCOS_COS_OBJ_PARSER_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosTypes.h>
#include <libcos/parse/CosParserOptions.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * The object types permitted in a given parsing position.
 *
 * A context carries a bitmask of these flags; a handler rejects its object
 * unless the corresponding flag is set. See @c cos_obj_parser_set_top_level_flags_.
 */
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

/**
 * Frees an object parser.
 *
 * @param parser The object parser.
 */
void
cos_obj_parser_destroy(CosObjParser *parser)
    COS_DEALLOCATOR_FUNC;

/**
 * @brief Allocate a new object parser.
 *
 * The options are copied, so @p options does not need to outlive the parser.
 *
 * @param document The document.
 * @param input_stream The input stream.
 * @param options The parser options, or @c NULL to use the defaults.
 *
 * @return The object parser, or @c NULL if an error occurred.
 */
CosObjParser * COS_Nullable
cos_obj_parser_create(CosDoc *document,
                      CosStream *input_stream,
                      const CosParserOptions * COS_Nullable options)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_obj_parser_destroy);

/**
 * @brief Allocate a new object parser that borrows an existing tokenizer.
 *
 * The parser does not take ownership of the tokenizer; the caller retains ownership.
 * This allows sharing a single tokenizer across multiple parsers.
 *
 * The options are copied, so @p options does not need to outlive the parser.
 * The tokenizer keeps the options it was created with.
 *
 * @param document The document.
 * @param tokenizer The tokenizer to use. Not owned by the parser.
 * @param options The parser options, or @c NULL to use the defaults.
 *
 * @return The object parser, or @c NULL if an error occurred.
 */
CosObjParser * COS_Nullable
cos_obj_parser_create_with_tokenizer(CosDoc *document,
                                     CosTokenizer *tokenizer,
                                     const CosParserOptions * COS_Nullable options)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_obj_parser_destroy);

/**
 * @brief Flushes the token buffer of the object parser.
 *
 * Call this after seeking the stream and resetting the tokenizer, to discard
 * any tokens that were buffered from the previous stream position, along with a
 * peeked object parsed there.
 *
 * @param parser The object parser.
 */
void
cos_obj_parser_flush_tokens_(CosObjParser *parser);

/**
 * Sets the object types permitted for the top-level object.
 *
 * The parser applies these flags to the object returned by
 * @c cos_obj_parser_peek_object and @c cos_obj_parser_next_object. Callers that
 * expect a specific shape at the top level -- an indirect definition, a trailer
 * dictionary, a bare direct object -- narrow the default with this before
 * parsing. The default is @c CosObjParserFlag_DirectObj combined with
 * @c CosObjParserFlag_IndirectObjDef.
 *
 * @param parser The object parser.
 * @param flags The permitted object types.
 */
void
cos_obj_parser_set_top_level_flags_(CosObjParser *parser,
                                    CosObjParserFlags flags);

/**
 * Checks if there is a next object or if the end of the input stream has been reached.
 *
 * @param parser The object parser.
 *
 * @return @c true if there is a next object, otherwise @c false.
 */
bool
cos_obj_parser_has_next_object(CosObjParser *parser);

/**
 * Peek the next object without consuming it.
 *
 * @param parser The object parser.
 * @param error On input, a pointer to an error object, or @c NULL.
 * On output, if an error occurred, the error object will be set with the error information.
 *
 * @return The next object, or @c NULL if an error occurred.
 */
CosObjNode * COS_Nullable
cos_obj_parser_peek_object(CosObjParser *parser,
                           CosError * COS_Nullable error);

/**
 * Get the next object and consume it.
 *
 * @param parser The object parser.
 * @param error On input, a pointer to an error object, or @c NULL.
 * On output, if an error occurred, the error object will be set with the error information.
 *
 * @return The next object, or @c NULL if an error occurred.
 */
CosObjNode * COS_Nullable
cos_obj_parser_next_object(CosObjParser *parser,
                           CosError * COS_Nullable error)
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_OBJ_PARSER_H */
