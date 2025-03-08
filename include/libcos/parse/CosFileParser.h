/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_PARSE_COS_PARSER_H
#define LIBCOS_PARSE_COS_PARSER_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief Deallocates a file parser object.
 *
 * @param parser The file parser object to deallocate.
 */
void
cos_file_parser_destroy(CosFileParser *parser)
    COS_DEALLOCATOR_FUNC;

CosFileParser * COS_Nullable
cos_file_parser_create(CosDoc *doc,
                       CosStream *input_stream)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_file_parser_destroy);

bool
cos_file_parser_parse(CosFileParser *parser,
                      CosError * COS_Nullable out_error);

COS_ASSUME_NONNULL_END COS_DECLS_END

#endif /* LIBCOS_PARSE_COS_PARSER_H */
