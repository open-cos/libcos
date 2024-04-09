//
// Created by david on 29/06/23.
//

#ifndef LIBCOS_COS_OBJ_PARSER_H
#define LIBCOS_COS_OBJ_PARSER_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosTypes.h>
#include <libcos/io/CosInputStream.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

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
 * @param document The document.
 * @param input_stream The input stream.
 *
 * @return The object parser, or @c NULL if an error occurred.
 */
CosObjParser * COS_Nullable
cos_obj_parser_create(CosDoc *document,
                      CosInputStream *input_stream)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_obj_parser_destroy);

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
CosObj * COS_Nullable
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
CosObj * COS_Nullable
cos_obj_parser_next_object(CosObjParser *parser,
                           CosError * COS_Nullable error)
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_OBJ_PARSER_H */
