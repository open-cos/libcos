/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_PARSE_COS_BASE_PARSER_H
#define LIBCOS_PARSE_COS_BASE_PARSER_H

#include "common/CosAPI.h"
#include "syntax/tokenizer/CosToken.h"

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosBaseParser {
    CosDoc *doc;             ///< The document being parsed.
    CosStream *input_stream; ///< The input stream to read from.
    CosTokenizer *tokenizer; ///< The tokenizer used for parsing.

    CosToken *token_buffer; ///< Buffer for tokens.
    size_t token_count;     ///< Number of tokens in the buffer.

    CosDiagnosticHandler * COS_Nullable diagnostic_handler; ///< Handler for diagnostics.
};

COS_API bool
cos_base_parser_init(CosBaseParser *parser,
                     CosDoc *document,
                     CosStream *input_stream)
    COS_OWNERSHIP_HOLDS(3)
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_PARSE_COS_BASE_PARSER_H */
