//
// Created by david on 18/05/23.
//

#ifndef LIBCOS_COS_DOC_H
#define LIBCOS_COS_DOC_H

#include <libcos/CosObjID.h>
#include <libcos/common/CosTypes.h>

CosDoc *
cos_doc_alloc(void);

void
cos_doc_free(CosDoc *doc);

int
cos_doc_get_version(CosDoc *doc);

CosObj *
cos_doc_get_root(CosDoc *doc);

void * COS_Nullable
cos_doc_get_object(CosDoc *doc,
                   CosObjID id,
                   CosError * COS_Nullable error);

#pragma mark - Diagnostics

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

#endif /* LIBCOS_COS_DOC_H */
