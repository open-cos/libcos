/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/CosIndirectObj.h"

#include "common/Assert.h"

COS_ASSUME_NONNULL_BEGIN

CosIndirectObj *
cos_indirect_obj_alloc(CosObjID id,
                       CosDoc * COS_Nullable document)
{
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
                      CosDoc * COS_Nullable document)
{
    COS_PARAMETER_ASSERT(indirect_obj != NULL);
    if (!indirect_obj) {
        return;
    }

    indirect_obj->id = id;
}

COS_ASSUME_NONNULL_END
