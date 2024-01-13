/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/objects/CosIndirectObj.h"

#include "common/Assert.h"
#include "libcos/CosBaseObj.h"
#include "libcos/private/objects/CosIndirectObj-Impl.h"

#include <libcos/CosDoc.h>
#include <libcos/objects/CosNullObj.h>

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

static void
cos_indirect_obj_resolve_value_(CosIndirectObj *indirect_obj);

CosIndirectObj *
cos_indirect_obj_alloc(CosObjID id,
                       CosDoc *document)
{
    COS_PARAMETER_ASSERT(document != NULL);

    CosIndirectObj *indirect_obj = cos_obj_alloc(sizeof(CosIndirectObj),
                                                 CosObjectType_Unknown,
                                                 document);
    if (!indirect_obj) {
        return NULL;
    }

    cos_indirect_obj_init(indirect_obj, id, document);

    return indirect_obj;
}

void
cos_indirect_obj_init(CosIndirectObj *indirect_obj,
                      CosObjID id,
                      CosDoc *document)
{
    COS_PARAMETER_ASSERT(indirect_obj != NULL);
    if (!indirect_obj) {
        return;
    }

    indirect_obj->id = id;
    indirect_obj->doc = document;
}

CosDirectObj *
cos_indirect_obj_get_value(CosIndirectObj *indirect_obj)
{
    COS_PARAMETER_ASSERT(indirect_obj != NULL);
    if (!indirect_obj) {
        return NULL;
    }

    if (!indirect_obj->value) {
        cos_indirect_obj_resolve_value_(indirect_obj);
    }

    return indirect_obj->value;
}

CosObjectType
cos_indirect_obj_get_type(CosIndirectObj *indirect_obj)
{
    COS_PARAMETER_ASSERT(indirect_obj != NULL);
    if (!indirect_obj) {
        return CosObjectType_Unknown;
    }

    if (!indirect_obj->value) {
        cos_indirect_obj_resolve_value_(indirect_obj);
    }

    const CosDirectObj * const direct_obj = indirect_obj->value;
    if (!direct_obj) {
        return CosObjectType_Unknown;
    }
    return cos_direct_obj_get_type(direct_obj);
}

#pragma mark - Private

static void
cos_indirect_obj_resolve_value_(CosIndirectObj *indirect_obj)
{
    COS_PARAMETER_ASSERT(indirect_obj != NULL);
    if (!indirect_obj) {
        return;
    }

    if (indirect_obj->value) {
        return;
    }

    CosError error = cos_error_none();
    void * const obj_value = cos_doc_get_object(indirect_obj->doc,
                                                indirect_obj->id,
                                                &error);
    if (obj_value) {
        indirect_obj->value = obj_value;
    }
    else {
        // TODO: Log undefined object if strict.

        indirect_obj->value = (CosDirectObj *)cos_null_obj_get();
    }
}

COS_ASSUME_NONNULL_END
