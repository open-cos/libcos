/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_COS_PARSER_H
#define LIBCOS_COS_PARSER_H

#include <libcos/common/CosBasicTypes.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief Deallocates a parser.
 *
 * @note Normally called by @c cos_doc_destroy(); users do not need to call this
 * directly if the document was created via @c cos_parser_create().
 *
 * @param parser The parser to deallocate.
 */
void
cos_parser_destroy(CosParser *parser)
    COS_DEALLOCATOR_FUNC;

/**
 * @brief Creates a new parser and registers it with the document.
 *
 * The created parser is immediately owned by @p document; @c cos_doc_destroy()
 * will free it. The returned pointer is a borrowed reference valid for the
 * lifetime of @p document.
 *
 * The parser borrows @p input_stream; the caller retains ownership and must
 * keep the stream alive until the document is destroyed.
 *
 * @param document The document to parse into.
 * @param input_stream The input stream to read PDF data from.
 *
 * @return A borrowed reference to the new parser, or @c NULL if an error occurred.
 */
CosParser * COS_Nullable
cos_parser_create(CosDoc *document,
                  CosStream *input_stream)
    COS_OWNERSHIP_HOLDS(2);

/**
 * @brief Parses the full PDF file structure from the input stream.
 *
 * Reads the header, locates the cross-reference table, parses xref entries
 * and the trailer dictionary, and stores the results in the document.
 *
 * @param parser The parser.
 * @param out_error On failure, set to describe the error.
 *
 * @return @c true on success, @c false if a parse error occurred.
 */
bool
cos_parser_parse(CosParser *parser,
                 CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(2);

/**
 * @brief Seeks to a byte offset and parses the next PDF object.
 *
 * Used by @c cos_doc_get_object() to load an indirect object on demand.
 *
 * @param parser The parser.
 * @param byte_offset Absolute byte offset of the object within the stream.
 * @param out_error On failure, set to describe the error.
 *
 * @return The parsed object (caller owns it), or @c NULL on error.
 */
CosObjNode * COS_Nullable
cos_parser_load_object(CosParser *parser,
                       CosStreamOffset byte_offset,
                       CosError * COS_Nullable out_error)
    COS_OWNERSHIP_RETURNS
    COS_ATTR_ACCESS_WRITE_ONLY(3);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_PARSER_H */
