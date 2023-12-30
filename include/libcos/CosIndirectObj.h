/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COS_INDIRECT_OBJ_H
#define LIBCOS_COS_INDIRECT_OBJ_H

#include <libcos/CosBaseObj.h>
#include <libcos/CosObjID.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>
#include <libcos/private/objects/CosBaseObj-Impl.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosIndirectObj {
    CosBaseObj base;

    /**
     * The ID of the indirect object.
     */
    CosObjID id;
};

CosIndirectObj * COS_Nullable
cos_indirect_obj_alloc(CosObjID id,
                       CosDocument * COS_Nullable document)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_indirect_obj_init(CosIndirectObj *indirect_obj,
                      CosObjID id,
                      CosDocument * COS_Nullable document);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_INDIRECT_OBJ_H */
