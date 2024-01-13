/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/CosObject.h"

#include "common/Assert.h"

#include <libcos/objects/CosDirectObj.h>
#include <libcos/objects/CosIndirectObj.h>

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

struct CosObject {
    enum {
        CosObjectValueType_Unknown = 0,

        CosObjectValueType_Direct,
        CosObjectValueType_Indirect,
    } type;

    union {
        CosDirectObj *direct_obj;
        CosIndirectObj *indirect_obj;
    } value;
};

CosObjectType
cos_object_get_type(CosObject *obj)
{
    COS_PARAMETER_ASSERT(obj != NULL);
    if (!obj) {
        return CosObjectType_Unknown;
    }

    switch (obj->type) {
        case CosObjectValueType_Unknown:
            return CosObjectType_Unknown;

        case CosObjectValueType_Direct:
            return cos_direct_obj_get_type(obj->value.direct_obj);
        case CosObjectValueType_Indirect:
            return cos_indirect_obj_get_type(obj->value.indirect_obj);
    }

    return CosObjectType_Unknown;
}

bool
cos_object_is_direct(const CosObject *obj)
{
    COS_PARAMETER_ASSERT(obj != NULL);
    if (!obj) {
        return false;
    }

    return (obj->type == CosObjectValueType_Direct);
}

bool
cos_object_is_indirect(const CosObject *obj)
{
    COS_PARAMETER_ASSERT(obj != NULL);
    if (!obj) {
        return false;
    }

    return (obj->type == CosObjectValueType_Indirect);
}

COS_ASSUME_NONNULL_END
