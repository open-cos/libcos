/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COS_DOC_H
#define LIBCOS_COS_DOC_H

#include "common/CosAPI.h"

#include <libcos/CosObjID.h>
#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>
#include <libcos/parse/CosParserOptions.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

COS_API void
cos_doc_destroy(CosDoc *doc)
    COS_DEALLOCATOR_FUNC;

COS_API CosDoc * COS_Nullable
cos_doc_create(void)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_doc_destroy);

COS_API int
cos_doc_get_version(CosDoc *doc);

COS_API CosObjNode * COS_Nullable
cos_doc_get_root(CosDoc *doc);

/**
 * Returns the document's newest cross-reference trailer.
 *
 * Older revisions are reached by walking @c cos_trailer_get_prev along the @c /Prev chain.
 *
 * @param doc The document.
 *
 * @return The newest trailer, or @c NULL if the document has not been parsed.
 */
COS_API CosTrailer * COS_Nullable
cos_doc_get_trailer(const CosDoc *doc);

COS_API void * COS_Nullable
cos_doc_get_object(CosDoc *doc,
                   CosObjID obj_id,
                   CosError * COS_Nullable error);

// MARK: - Diagnostics

/**
 * @brief Gets the document's diagnostic handler.
 *
 * @param doc The document.
 *
 * @return The document's diagnostic handler, or @c NULL if no diagnostic handler is set.
 */
COS_API CosDiagnosticHandler * COS_Nullable
cos_doc_get_diagnostic_handler(const CosDoc *doc);

COS_API void
cos_doc_set_diagnostic_handler(CosDoc *doc,
                               CosDiagnosticHandler * COS_Nullable handler);

// MARK: - Parser options

/**
 * @brief Gets the options the document is being parsed with.
 *
 * Set by @c cos_parser_create() . Consumers that resolve objects without a
 * parser in scope read the options from here.
 *
 * @param doc The document.
 *
 * @return The document's parser options, or the defaults if @p doc is @c NULL .
 */
COS_API CosParserOptions
cos_doc_get_parser_options(const CosDoc *doc);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_DOC_H */
