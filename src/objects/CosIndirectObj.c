/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/objects/CosIndirectObj.h"

#include "common/Assert.h"
#include "libcos/objects/CosObj.h"

#include <libcos/CosDoc.h>

#include <stddef.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosIndirectObj {
    /**
     * @c CosObjType_Indirect.
     */
    CosObjType type;

    /**
     * The ID of the indirect object.
     */
    CosObjID id;

    /**
     * The (direct) object value.
     */
    CosObj *value;
};

CosIndirectObj *
cos_indirect_obj_alloc(CosObjID id,
                       CosObj *value)
{
    COS_PARAMETER_ASSERT(value != NULL);
    if (!value) {
        return NULL;
    }

    CosIndirectObj * const indirect_obj = calloc(1, sizeof(CosIndirectObj));
    if (!indirect_obj) {
        return NULL;
    }

    indirect_obj->type = CosObjType_Indirect;
    indirect_obj->id = id;
    indirect_obj->value = value;

    return indirect_obj;
}

void
cos_indirect_obj_free(CosIndirectObj *indirect_obj)
{
    if (!indirect_obj) {
        return;
    }

    cos_obj_free(indirect_obj->value);

    free(indirect_obj);
}

CosObjID
cos_indirect_obj_get_id(const CosIndirectObj *indirect_obj)
{
    COS_PARAMETER_ASSERT(indirect_obj != NULL);
    if (!indirect_obj) {
        return CosObjID_Invalid;
    }

    return indirect_obj->id;
}

CosObj *
cos_indirect_obj_get_value(const CosIndirectObj *indirect_obj)
{
    COS_PARAMETER_ASSERT(indirect_obj != NULL);
    if (!indirect_obj) {
        return NULL;
    }

    return indirect_obj->value;
}

CosObjValueType
cos_indirect_obj_get_type(const CosIndirectObj *indirect_obj)
{
    COS_PARAMETER_ASSERT(indirect_obj != NULL);
    if (!indirect_obj) {
        return CosObjValueType_Unknown;
    }

    const CosObj * const direct_obj = indirect_obj->value;
    if (!direct_obj) {
        return CosObjValueType_Unknown;
    }
    return (CosObjValueType)cos_obj_get_type(direct_obj);
}

COS_ASSUME_NONNULL_END
