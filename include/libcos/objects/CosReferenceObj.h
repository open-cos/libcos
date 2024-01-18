/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_OBJECTS_COS_REFERENCE_OBJ_H
#define LIBCOS_OBJECTS_COS_REFERENCE_OBJ_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>
#include <libcos/objects/CosObjectTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

CosReferenceObj * COS_Nullable
cos_reference_obj_alloc(CosObjID id,
                        CosDoc *document)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_reference_obj_free(CosReferenceObj *reference_obj);

CosObjValueType
cos_reference_obj_get_type(CosReferenceObj *reference_obj)
    COS_WARN_UNUSED_RESULT;

CosObj * COS_Nullable
cos_reference_obj_get_value(CosReferenceObj *reference_obj)
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_OBJECTS_COS_REFERENCE_OBJ_H */
