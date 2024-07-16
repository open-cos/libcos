/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/CosDoc.h"

#include "common/Assert.h"
#include "libcos/common/memory/CosAllocator.h"

#include <libcos/common/memory/CosMemory.h>

#include <string.h>

COS_ASSUME_NONNULL_BEGIN

struct CosDoc {
    int version;

    CosAllocator *allocator;
    CosObj * COS_Nullable root;

    CosDiagnosticHandler * COS_Nullable diagnostic_handler;
};

CosDoc *
cos_doc_create(CosAllocator * COS_Nullable allocator)
{
    CosAllocator * const doc_allocator = allocator ? allocator : CosAllocatorDefault;

    CosDoc * const doc = cos_alloc(doc_allocator,
                                   sizeof(CosDoc));
    if (!doc) {
        return NULL;
    }

    memset(doc, 0, sizeof(CosDoc));

    doc->allocator = doc_allocator;

    return doc;
}

void
cos_doc_destroy(CosDoc *doc)
{
    if (!doc) {
        return;
    }

    cos_free(doc->allocator, doc);
}

int
cos_doc_get_version(CosDoc *doc)
{
    if (!doc) {
        return 0;
    }

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

// MARK: - Diagnostics

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
