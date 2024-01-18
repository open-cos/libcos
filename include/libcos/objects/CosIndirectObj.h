/*
 * Copyright (c) 2023-2024 OpenCOS.
 */

#ifndef LIBCOS_COS_INDIRECT_OBJ_H
#define LIBCOS_COS_INDIRECT_OBJ_H

#include "libcos/objects/CosObjectTypes.h"

#include <libcos/CosObjID.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

CosIndirectObj * COS_Nullable
cos_indirect_obj_alloc(CosObjID id,
                       CosObj *value)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_indirect_obj_free(CosIndirectObj *indirect_obj);

CosObjID
cos_indirect_obj_get_id(const CosIndirectObj *indirect_obj);

/**
 * @brief Gets the value of the indirect object.
 *
 * @param indirect_obj The indirect object.
 *
 * @return The value of the indirect object, or @c NULL if the object could not
 * be resolved.
 */
CosObj * COS_Nullable
cos_indirect_obj_get_value(const CosIndirectObj *indirect_obj)
    COS_WARN_UNUSED_RESULT;

/**
 * @brief Gets the indirect object's type.
 *
 * @param indirect_obj The indirect object.
 *
 * @return The indirect object's type, or @c CosObjValueType_Unknown if the object
 * could not be resolved.
 */
CosObjValueType
cos_indirect_obj_get_type(const CosIndirectObj *indirect_obj)
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_INDIRECT_OBJ_H */
