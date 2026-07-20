/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/common/memory/CosMemory.h"
#include "libcos/objects/CosReferenceObjNode.h"

#include "common/Assert.h"

#include "libcos/CosDoc.h"
#include "libcos/CosObjID.h"
#include "libcos/common/CosError.h"
#include "libcos/objects/CosNullObjNode.h"
#include "libcos/objects/CosObjNode.h"
#include "parse/CosParserOptions-Private.h"

#include <stdio.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosReferenceObjNode {
    CosObjNodeType type;
    unsigned int ref_count;

    CosObjID id;
    CosDoc *doc;
    CosObjNode * COS_Nullable value;
};

static void
cos_reference_obj_node_resolve_value_(CosReferenceObjNode *reference_obj);

CosReferenceObjNode * COS_Nullable
cos_reference_obj_node_alloc(CosObjID id,
                             CosDoc *document)
{
    COS_API_PARAM_CHECK(document != NULL);

    CosReferenceObjNode * const reference_obj = cos_calloc(1, sizeof(CosReferenceObjNode));
    if (!reference_obj) {
        return NULL;
    }

    reference_obj->type = CosObjNodeType_Reference;
    reference_obj->ref_count = 1;
    reference_obj->id = id;
    reference_obj->doc = document;

    return reference_obj;
}

void
cos_reference_obj_node_free(CosReferenceObjNode *reference_obj)
{
    if (!reference_obj) {
        return;
    }

    // reference_obj->value is a borrowed pointer owned by the document's object cache, so it is
    // not released here (see cos_reference_obj_node_resolve_value_).
    cos_free(reference_obj);
}

CosObjID
cos_reference_obj_node_get_id(const CosReferenceObjNode *reference_obj)
{
    COS_API_PARAM_CHECK(reference_obj != NULL);
    if (COS_UNLIKELY(!reference_obj)) {
        return CosObjID_Invalid;
    }

    return reference_obj->id;
}

CosObjNode *
cos_reference_obj_node_get_value(CosReferenceObjNode *reference_obj)
{
    COS_API_PARAM_CHECK(reference_obj != NULL);
    if (!reference_obj) {
        return NULL;
    }

    if (!reference_obj->value) {
        cos_reference_obj_node_resolve_value_(reference_obj);
    }

    return reference_obj->value;
}

CosObjNodeValueType
cos_reference_obj_node_get_type(CosReferenceObjNode *reference_obj)
{
    COS_API_PARAM_CHECK(reference_obj != NULL);
    if (!reference_obj) {
        return CosObjNodeValueType_Unknown;
    }

    if (!reference_obj->value) {
        cos_reference_obj_node_resolve_value_(reference_obj);
    }

    const CosObjNode * const direct_obj = reference_obj->value;
    if (!direct_obj) {
        return CosObjNodeValueType_Unknown;
    }
    return (CosObjNodeValueType)cos_obj_node_get_type(direct_obj);
}

// MARK: - Private

static void
cos_reference_obj_node_resolve_value_(CosReferenceObjNode *reference_obj)
{
    COS_IMPL_PARAM_CHECK(reference_obj != NULL);
    if (!reference_obj) {
        return;
    }

    if (reference_obj->value) {
        return;
    }

    CosError error = cos_error_none();
    void * const obj_value = cos_doc_get_object(reference_obj->doc,
                                                reference_obj->id,
                                                &error);
    if (obj_value) {
        // The document's object cache owns the resolved object for the lifetime of the
        // document. Hold a borrowed pointer and drop the reference returned by
        // cos_doc_get_object, so that cyclic references (such as a page's /Parent pointing back
        // to its /Pages node) do not form ownership cycles that outlive the cache and leak.
        reference_obj->value = obj_value;
        cos_obj_node_release((CosObjNode *)obj_value);
    }
    else {
        const CosParserOptions options =
            cos_doc_get_parser_options(reference_obj->doc);

        (void)cos_options_report_(&options,
                                  cos_doc_get_diagnostic_handler(reference_obj->doc),
                                  CosStrictGroup_UndefinedRefs,
                                  "Indirect reference does not resolve to an object",
                                  NULL);

        // Resolution stays total even at CosStrictLevel_Error: this function
        // returns void and has no error channel, so the level can change how
        // loudly the deviation is reported but not whether it fails. Callers
        // that need to reject an unresolved reference watch the diagnostic.
        reference_obj->value = (CosObjNode *)cos_null_obj_node_get();
    }
}

void
cos_reference_obj_node_print_desc(const CosReferenceObjNode *reference_obj)
{
    COS_API_PARAM_CHECK(reference_obj != NULL);
    if (!reference_obj) {
        return;
    }

    printf("Indirect reference: %u %u\n",
           reference_obj->id.obj_number,
           reference_obj->id.gen_number);
}

COS_ASSUME_NONNULL_END
