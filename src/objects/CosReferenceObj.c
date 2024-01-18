/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/objects/CosReferenceObj.h"

#include "common/Assert.h"
#include "libcos/CosDoc.h"
#include "libcos/CosObjID.h"
#include "libcos/common/CosError.h"
#include "libcos/objects/CosNullObj.h"
#include "libcos/objects/CosObj.h"

#include <stdlib.h>

struct CosReferenceObj {
    CosObjType type;

    CosObjID id;
    CosDoc *doc;
    CosObj *value;
};

static void
cos_reference_obj_resolve_value_(CosReferenceObj *reference_obj);

CosReferenceObj * COS_Nullable
cos_reference_obj_alloc(CosObjID id,
                        CosDoc *document)
{
    COS_PARAMETER_ASSERT(document != NULL);

    CosReferenceObj * const reference_obj = calloc(1, sizeof(CosReferenceObj));
    if (!reference_obj) {
        return NULL;
    }

    reference_obj->type = CosObjType_Reference;
    reference_obj->id = id;
    reference_obj->doc = document;

    return reference_obj;
}

void
cos_reference_obj_free(CosReferenceObj *reference_obj)
{
    if (!reference_obj) {
        return;
    }

    // TODO: Decrease reference count of value.

    free(reference_obj);
}

CosObj *
cos_reference_obj_get_value(CosReferenceObj *reference_obj)
{
    COS_PARAMETER_ASSERT(reference_obj != NULL);
    if (!reference_obj) {
        return NULL;
    }

    if (!reference_obj->value) {
        cos_reference_obj_resolve_value_(reference_obj);
    }

    return reference_obj->value;
}

CosObjValueType
cos_reference_obj_get_type(CosReferenceObj *reference_obj)
{
    COS_PARAMETER_ASSERT(reference_obj != NULL);
    if (!reference_obj) {
        return CosObjValueType_Unknown;
    }

    if (!reference_obj->value) {
        cos_reference_obj_resolve_value_(reference_obj);
    }

    const CosObj * const direct_obj = reference_obj->value;
    if (!direct_obj) {
        return CosObjValueType_Unknown;
    }
    return (CosObjValueType)cos_obj_get_type(direct_obj);
}

#pragma mark - Private

static void
cos_reference_obj_resolve_value_(CosReferenceObj *reference_obj)
{
    COS_PARAMETER_ASSERT(reference_obj != NULL);
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

        reference_obj->value = (CosObj *)cos_null_obj_get();
    }
}
