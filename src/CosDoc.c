/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/CosDoc.h"

#include "common/Assert.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosDoc {
    int version;
    int x;

    CosObj *root;

    CosDiagnosticHandler * COS_Nullable diagnostic_handler;
};

CosDoc *
cos_doc_alloc(void)
{
    CosDoc * const doc = calloc(1, sizeof(CosDoc));
    return doc;
}

void
cos_doc_free(CosDoc *doc)
{
    free(doc);
}

int
cos_doc_get_version(CosDoc *doc)
{
    return doc->version;
}

CosObj *
cos_doc_get_root(CosDoc *doc)
{
    return doc->root;
}

void *
cos_doc_get_object(CosDoc *doc,
                   CosObjID id,
                   CosError * COS_Nullable error)
{
    (void)doc;
    (void)id;
    (void)error;

    return NULL;
}

#pragma mark - Diagnostics

CosDiagnosticHandler *
cos_doc_get_diagnostic_handler(const CosDoc *doc)
{
    COS_PARAMETER_ASSERT(doc != NULL);
    if (!doc) {
        return NULL;
    }

    return doc->diagnostic_handler;
}

void
cos_doc_set_diagnostic_handler(CosDoc *doc,
                               CosDiagnosticHandler * COS_Nullable handler)
{
    COS_PARAMETER_ASSERT(doc != NULL);
    if (!doc) {
        return;
    }

    doc->diagnostic_handler = handler;
}

COS_ASSUME_NONNULL_END
