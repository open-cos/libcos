/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/CosDoc.h"

#include "CosDoc-Private.h"
#include "common/Assert.h"
#include "objects/CosObjCache.h"
#include "parse/CosParserOptions-Private.h"

#include <libcos/CosObjID.h>
#include <libcos/CosParser.h>
#include <libcos/CosTrailer.h>
#include <libcos/common/CosError.h>
#include <libcos/common/memory/CosAllocator.h>
#include <libcos/common/memory/CosMemory.h>
#include <libcos/objects/CosDictObjNode.h>
#include <libcos/objects/CosIndirectObjNode.h>
#include <libcos/objects/CosObjNode.h>
#include <libcos/objects/CosObjStream.h>
#include <libcos/objects/CosStreamObjNode.h>
#include <libcos/parse/CosParserOptions.h>
#include <libcos/xref/table/CosXrefEntry.h>
#include <libcos/xref/table/CosXrefTable.h>

#include <string.h>

COS_ASSUME_NONNULL_BEGIN

struct CosDoc {
    int version;

    CosAllocator *allocator;
    CosObjNode * COS_Nullable root;

    CosParser * COS_Nullable parser;
    CosXrefTable * COS_Nullable xref_table;
    CosTrailer * COS_Nullable trailer;

    CosObjCache * COS_Nullable obj_cache;

    CosDiagnosticHandler * COS_Nullable diagnostic_handler;

    /**
     * The options the document was parsed with.
     *
     * Held here for consumers that have a document but no parser in scope,
     * such as reference resolution.
     */
    CosParserOptions parser_options;
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

    // Not left zeroed: an all-zero CosParserOptions would silence every group,
    // whereas the documented default is to warn.
    doc->parser_options = cos_parser_options_make_default();

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

    if (doc->trailer) {
        cos_trailer_destroy(COS_nonnull_cast(doc->trailer));
        doc->trailer = NULL;
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

CosObjNode *
cos_doc_get_root(CosDoc *doc)
{
    return doc->root;
}

static CosObjNode * COS_Nullable
cos_doc_load_compressed_(CosDoc *doc,
                         CosObjID obj_id,
                         const CosXrefEntry *entry,
                         CosError * COS_Nullable error);

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
            // Compressed objects always have generation number zero, so a
            // reference carrying any other generation is malformed. Adobe
            // ignores the generation number here rather than failing, so by
            // default the reference still resolves.
            if (obj_id.gen_number != 0) {
                if (!cos_options_report_(&(doc->parser_options),
                                         doc->diagnostic_handler,
                                         CosStrictGroup_CompressedObjGen,
                                         "Non-zero generation number on a compressed object",
                                         error)) {
                    return NULL;
                }
            }
            break;

        case CosXrefEntryType_InUse:
            // Validate that the reference's generation number matches the xref entry.
            if (entry->value.in_use.gen_number != obj_id.gen_number) {
                cos_error_propagate(error,
                                    cos_error_make(COS_ERROR_XREF,
                                                   "Generation number mismatch"));
                return NULL;
            }
            break;
    }

    // Check the cache for a previously loaded object. This serves both entry types, and is what
    // makes eager object-stream population pay off: after the first compressed object of a
    // stream is loaded, its siblings (and the object stream itself) are cache hits.
    if (doc->obj_cache) {
        CosObjNode * const cached = cos_obj_cache_get(COS_nonnull_cast(doc->obj_cache),
                                                  obj_id.obj_number);
        if (cached) {
            return cos_obj_node_retain(cached);
        }
    }

    if (entry->type == CosXrefEntryType_Compressed) {
        return cos_doc_load_compressed_(doc, obj_id, entry, error);
    }

    // Parse the in-use object from the file.
    CosObjNode * const obj = cos_parser_load_object(COS_nonnull_cast(doc->parser),
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

// Resolves a compressed (type-2) object by decoding its object stream, eagerly parsing every
// object it contains into the document cache, and returning the requested one.
static CosObjNode *
cos_doc_load_compressed_(CosDoc *doc,
                         CosObjID obj_id,
                         const CosXrefEntry *entry,
                         CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(doc != NULL);
    COS_IMPL_PARAM_CHECK(entry != NULL);

    const unsigned int stream_number = entry->value.compressed.obj_stream_number;

    // The object stream is itself an ordinary in-use object; fetching it caches it and returns a
    // retained reference.
    CosObjNode * const objstm_obj =
        cos_doc_get_object(doc, cos_obj_id_make(stream_number, 0), error);
    if (!objstm_obj) {
        return NULL;
    }

    if (!cos_obj_node_is_indirect(objstm_obj)) {
        cos_obj_node_release(objstm_obj);
        cos_error_propagate(error,
                            cos_error_make(COS_ERROR_XREF,
                                           "Compressed object's container is not an indirect object"));
        return NULL;
    }

    CosObjNode * const value = cos_indirect_obj_node_get_value((CosIndirectObjNode *)objstm_obj);
    if (!value || !cos_obj_node_is_stream(value)) {
        cos_obj_node_release(objstm_obj);
        cos_error_propagate(error,
                            cos_error_make(COS_ERROR_XREF,
                                           "Compressed object's container is not an object stream"));
        return NULL;
    }

    // cos_obj_stream_create copies out the decoded bytes, so the object stream no longer depends
    // on the container node once created; the cache keeps the container alive regardless.
    CosObjStream * const obj_stream = cos_obj_stream_create((CosStreamObjNode *)value, doc, error);
    cos_obj_node_release(objstm_obj);
    if (!obj_stream) {
        return NULL;
    }

    if (!doc->obj_cache) {
        doc->obj_cache = cos_obj_cache_create(0);
    }
    if (!doc->obj_cache) {
        cos_obj_stream_destroy(obj_stream);
        cos_error_propagate(error,
                            cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to create object cache"));
        return NULL;
    }

    const size_t count = cos_obj_stream_get_count(obj_stream);
    bool ok = true;
    for (size_t i = 0; i < count; i++) {
        unsigned int obj_number = 0;
        if (!cos_obj_stream_get_obj_number_at(obj_stream, i, &obj_number, error)) {
            ok = false;
            break;
        }

        CosObjNode * const contained = cos_obj_stream_get_object_at(obj_stream, i, error);
        if (!contained) {
            ok = false;
            break;
        }

        cos_obj_cache_insert(COS_nonnull_cast(doc->obj_cache), obj_number, contained);
        cos_obj_node_release(contained);
    }

    cos_obj_stream_destroy(obj_stream);

    if (!ok) {
        return NULL;
    }

    CosObjNode * const result = cos_obj_cache_get(COS_nonnull_cast(doc->obj_cache),
                                                  obj_id.obj_number);
    if (!result) {
        cos_error_propagate(error,
                            cos_error_make(COS_ERROR_XREF,
                                           "Compressed object was not found in its object stream"));
        return NULL;
    }

    return cos_obj_node_retain(result);
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

// MARK: - Parser options

CosParserOptions
cos_doc_get_parser_options(const CosDoc *doc)
{
    COS_API_PARAM_CHECK(doc != NULL);
    if (COS_UNLIKELY(!doc)) {
        return cos_parser_options_make_default();
    }

    return doc->parser_options;
}

// MARK: - Private setters

void
cos_doc_set_parser_options_(CosDoc *doc,
                            const CosParserOptions * COS_Nullable options)
{
    COS_IMPL_PARAM_CHECK(doc != NULL);
    if (!doc) {
        return;
    }

    doc->parser_options = cos_parser_options_resolve_(options);
}

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
cos_doc_set_trailer_(CosDoc *doc,
                     CosTrailer * COS_Nullable trailer)
{
    if (!doc) {
        return;
    }

    if (doc->trailer) {
        cos_trailer_destroy(COS_nonnull_cast(doc->trailer));
    }

    doc->trailer = trailer;
}

CosTrailer *
cos_doc_get_trailer(const CosDoc *doc)
{
    COS_API_PARAM_CHECK(doc != NULL);
    if (COS_UNLIKELY(!doc)) {
        return NULL;
    }

    return doc->trailer;
}

void
cos_doc_set_root_(CosDoc *doc,
                  CosObjNode * COS_Nullable root)
{
    if (!doc) {
        return;
    }

    doc->root = root;
}

COS_ASSUME_NONNULL_END
