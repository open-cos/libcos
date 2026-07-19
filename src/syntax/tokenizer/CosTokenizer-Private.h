/*
 * Copyright (c) 2026 OpenCOS.
 */

#ifndef LIBCOS_SYNTAX_TOKENIZER_COS_TOKENIZER_PRIVATE_H
#define LIBCOS_SYNTAX_TOKENIZER_COS_TOKENIZER_PRIVATE_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * Redirects the tokenizer's diagnostics to a handler.
 *
 * A tokenizer is created before any document is in scope, so it starts out
 * reporting to the default handler. A parser that owns the tokenizer calls
 * this to route its diagnostics to the document's handler instead.
 *
 * @param tokenizer The tokenizer.
 * @param handler The handler to report to. Borrowed, not owned.
 */
void
cos_tokenizer_set_diagnostic_handler_(CosTokenizer *tokenizer,
                                      CosDiagnosticHandler *handler);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_SYNTAX_TOKENIZER_COS_TOKENIZER_PRIVATE_H */
