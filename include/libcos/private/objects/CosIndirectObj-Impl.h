/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_PRIVATE_OBJECTS_COS_INDIRECT_OBJ_IMPL_H
#define LIBCOS_PRIVATE_OBJECTS_COS_INDIRECT_OBJ_IMPL_H

#include "libcos/objects/CosIndirectObj.h"

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosIndirectObj {
    /**
     * The ID of the indirect object.
     */
    CosObjID id;

    /**
     * The document of the indirect object.
     */
    CosDoc *doc;

    /**
     * The object value.
     */
    CosDirectObj * COS_Nullable value;
};

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_PRIVATE_OBJECTS_COS_INDIRECT_OBJ_IMPL_H */
