/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/CosDoc.h"

#include "CosDoc-Private.h"
#include "common/Assert.h"
#include "objects/CosObjCache.h"

#include <libcos/CosObjID.h>
#include <libcos/CosParser.h>
#include <libcos/common/CosError.h>
#include <libcos/common/memory/CosAllocator.h>
#include <libcos/common/memory/CosMemory.h>
#include <libcos/objects/CosDictObj.h>
#include <libcos/objects/CosObj.h>
#include <libcos/xref/table/CosXrefEntry.h>
#include <libcos/xref/table/CosXrefTable.h>

#include <string.h>

COS_ASSUME_NONNULL_BEGIN

struct CosDoc {
    int version;

    CosAllocator *allocator;
    CosObj * COS_Nullable root;

    CosParser * COS_Nullable parser;
    CosXrefTable * COS_Nullable xref_table;
    CosDictObj * COS_Nullable trailer_dict;

    CosObjCache * COS_Nullable obj_cache;

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

    if (doc->parser) {
        cos_parser_destroy(COS_nonnull_cast(doc->parser));
        doc->parser = NULL;
    }

    if (doc->xref_table) {
        cos_xref_table_destroy(COS_nonnull_cast(doc->xref_table));
        doc->xref_table = NULL;
    }

    if (doc->trailer_dict) {
        cos_dict_obj_destroy(COS_nonnull_cast(doc->trailer_dict));
        doc->trailer_dict = NULL;
    }

    if (doc->obj_cache) {
        cos_obj_cache_destroy(COS_nonnull_cast(doc->obj_cache));
        doc->obj_cache = NULL;
    }

    cos_free(doc->allocator, doc);
}

CosAllocator *
cos_doc_get_allocator(const CosDoc *doc)
{
    COS_API_PARAM_CHECK(doc != NULL);
    if (COS_UNLIKELY(!doc)) {
        return NULL;
    }

    return doc->allocator;
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
                   CosObjID obj_id,
                   CosError * COS_Nullable error)
{
    COS_API_PARAM_CHECK(doc != NULL);
    if (COS_UNLIKELY(!doc)) {
        return NULL;
    }

    if (!doc->parser || !doc->xref_table) {
        cos_error_propagate(error,
                            cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Document has not been parsed"));
        return NULL;
    }

    const CosXrefEntry * const entry =
        cos_xref_table_find_entry_for_obj_num(COS_nonnull_cast(doc->xref_table),
                                              (CosObjNumber)obj_id.obj_number,
                                              error);
    if (!entry) {
        cos_error_propagate(error,
                            cos_error_make(COS_ERROR_XREF,
                                           "Object not found in xref table"));
        return NULL;
    }

    switch (entry->type) {
        case CosXrefEntryType_Free:
            cos_error_propagate(error,
                                cos_error_make(COS_ERROR_XREF,
                                               "Object is free"));
            return NULL;

        case CosXrefEntryType_Compressed:
            cos_error_propagate(error,
                                cos_error_make(COS_ERROR_NOT_IMPLEMENTED,
                                               "Compressed objects are not yet supported"));
            return NULL;

        case CosXrefEntryType_InUse:
            break;
    }

    // Validate that the reference's generation number matches the xref entry.
    if (entry->value.in_use.gen_number != obj_id.gen_number) {
        cos_error_propagate(error,
                            cos_error_make(COS_ERROR_XREF,
                                           "Generation number mismatch"));
        return NULL;
    }

    // Check the cache for a previously loaded object.
    if (doc->obj_cache) {
        CosObj * const cached = cos_obj_cache_get(COS_nonnull_cast(doc->obj_cache),
                                                  obj_id.obj_number);
        if (cached) {
            return cos_obj_retain(cached);
        }
    }

    // Parse the object from the file.
    CosObj * const obj = cos_parser_load_object(COS_nonnull_cast(doc->parser),
                                                (CosStreamOffset)entry->value.in_use.byte_offset,
                                                error);
    if (!obj) {
        return NULL;
    }

    // Lazily create the cache and insert the object.
    if (!doc->obj_cache) {
        doc->obj_cache = cos_obj_cache_create(0);
    }
    if (doc->obj_cache) {
        cos_obj_cache_insert(COS_nonnull_cast(doc->obj_cache),
                             obj_id.obj_number,
                             obj);
    }

    return obj;
}

// MARK: - Diagnostics

CosDiagnosticHandler *
cos_doc_get_diagnostic_handler(const CosDoc *doc)
{
    COS_API_PARAM_CHECK(doc != NULL);
    if (!doc) {
        return NULL;
    }

    return doc->diagnostic_handler;
}

void
cos_doc_set_diagnostic_handler(CosDoc *doc,
                               CosDiagnosticHandler * COS_Nullable handler)
{
    COS_API_PARAM_CHECK(doc != NULL);
    if (!doc) {
        return;
    }

    doc->diagnostic_handler = handler;
}

// MARK: - Private setters

void
cos_doc_set_version_(CosDoc *doc,
                     int version)
{
    if (!doc) {
        return;
    }

    doc->version = version;
}

void
cos_doc_set_parser_(CosDoc *doc,
                    CosParser * COS_Nullable parser)
{
    if (!doc) {
        return;
    }

    if (doc->parser) {
        cos_parser_destroy(COS_nonnull_cast(doc->parser));
    }

    doc->parser = parser;
}

void
cos_doc_set_xref_table_(CosDoc *doc,
                        CosXrefTable * COS_Nullable table)
{
    if (!doc) {
        return;
    }

    if (doc->xref_table) {
        cos_xref_table_destroy(COS_nonnull_cast(doc->xref_table));
    }

    doc->xref_table = table;
}

void
cos_doc_set_trailer_dict_(CosDoc *doc,
                          CosDictObj * COS_Nullable dict)
{
    if (!doc) {
        return;
    }

    if (doc->trailer_dict) {
        cos_dict_obj_destroy(COS_nonnull_cast(doc->trailer_dict));
    }

    doc->trailer_dict = dict;
}

void
cos_doc_set_root_(CosDoc *doc,
                  CosObj * COS_Nullable root)
{
    if (!doc) {
        return;
    }

    doc->root = root;
}

COS_ASSUME_NONNULL_END
