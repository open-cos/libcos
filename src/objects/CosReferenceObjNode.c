/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/objects/CosReferenceObjNode.h"

#include "common/Assert.h"

#include "libcos/CosDoc.h"
#include "libcos/CosObjID.h"
#include "libcos/common/CosError.h"
#include "libcos/objects/CosNullObjNode.h"
#include "libcos/objects/CosObjNode.h"

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

    CosReferenceObjNode * const reference_obj = calloc(1, sizeof(CosReferenceObjNode));
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

    if (reference_obj->value) {
        cos_obj_node_free(COS_nonnull_cast(reference_obj->value));
    }

    free(reference_obj);
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
        reference_obj->value = obj_value;
    }
    else {
        // TODO: Log undefined object if strict.

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
