//
// Created by david on 29/06/23.
//

#ifndef LIBCOS_COS_PARSER_H
#define LIBCOS_COS_PARSER_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * Frees an object parser.
 *
 * @param parser The object parser.
 */
void
cos_obj_parser_destroy2(CosObjParser *parser)
    COS_DEALLOCATOR_FUNC;

/**
 * @brief Allocate a new object parser.
 *
 * @param document The document.
 * @param input_stream The input stream.
 *
 * @return The object parser, or @c NULL if an error occurred.
 */
CosObjParser * COS_Nullable
cos_obj_parser_create2(CosDoc *document,
                      CosStream *input_stream)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_obj_parser_destroy2);

/**
 * Checks if there is a next object or if the end of the input stream has been reached.
 *
 * @param parser The object parser.
 *
 * @return @c true if there is a next object, otherwise @c false.
 */
bool
cos_obj_parser_has_next(CosObjParser *parser);

/**
 * Peek the next object without consuming it.
 *
 * @param parser The object parser.
 * @param error On input, a pointer to an error object, or @c NULL.
 * On output, if an error occurred, the error object will be set with the error information.
 *
 * @return The next object, or @c NULL if an error occurred.
 */
CosNode * COS_Nullable
cos_obj_parser_peek(CosObjParser *parser,
                    CosError * COS_Nullable error);

/**
 * Get and consume the next object.
 *
 * @param parser The object parser.
 * @param error On input, a pointer to an error object, or @c NULL.
 * On output, if an error occurred, the error object will be set with the error information.
 *
 * @return The next object, or @c NULL if an error occurred.
 */
CosNode * COS_Nullable
cos_obj_parser_next(CosObjParser *parser,
                    CosError * COS_Nullable error)
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_PARSER_H */
