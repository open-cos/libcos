/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COS_DOC_H
#define LIBCOS_COS_DOC_H

#include <libcos/CosObjID.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

void
cos_doc_destroy(CosDoc *doc)
    COS_DEALLOCATOR_FUNC;

CosDoc * COS_Nullable
cos_doc_create(CosAllocator * COS_Nullable allocator)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_doc_destroy);

bool
cos_doc_load(CosDoc *doc,
             CosStream *stream,
             CosError * COS_Nullable error);

int
cos_doc_get_version(CosDoc *doc);

CosObj * COS_Nullable
cos_doc_get_root(CosDoc *doc);

void * COS_Nullable
cos_doc_get_object(CosDoc *doc,
                   CosObjID id,
                   CosError * COS_Nullable error);

// MARK: - Diagnostics

/**
 * @brief Gets the document's diagnostic handler.
 *
 * @param doc The document.
 *
 * @return The document's diagnostic handler, or @c NULL if no diagnostic handler is set.
 */
CosDiagnosticHandler * COS_Nullable
cos_doc_get_diagnostic_handler(const CosDoc *doc);

void
cos_doc_set_diagnostic_handler(CosDoc *doc,
                               CosDiagnosticHandler * COS_Nullable handler);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_DOC_H */
