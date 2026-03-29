/*
 * Copyright (c) 2023-2024 OpenCOS.
 */

#ifndef LIBCOS_COS_INDIRECT_OBJ_H
#define LIBCOS_COS_INDIRECT_OBJ_H

#include "libcos/objects/CosObjNodeTypes.h"

#include <libcos/CosObjID.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

CosIndirectObjNode * COS_Nullable
cos_indirect_obj_node_alloc(CosObjID id,
                       CosObjNode *value)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_indirect_obj_node_free(CosIndirectObjNode *indirect_obj);

/**
 * @brief Gets the identifier of the indirect object.
 *
 * @param indirect_obj The indirect object.
 *
 * @return The identifier of the indirect object.
 */
CosObjID
cos_indirect_obj_node_get_id(const CosIndirectObjNode *indirect_obj);

/**
 * @brief Gets the value of the indirect object.
 *
 * @param indirect_obj The indirect object.
 *
 * @return The value of the indirect object, or @c NULL if the object could not
 * be resolved.
 */
CosObjNode * COS_Nullable
cos_indirect_obj_node_get_value(const CosIndirectObjNode *indirect_obj)
    COS_WARN_UNUSED_RESULT;

/**
 * @brief Gets the indirect object's type.
 *
 * @param indirect_obj The indirect object.
 *
 * @return The indirect object's type, or @c CosObjNodeValueType_Unknown if the object
 * could not be resolved.
 */
CosObjNodeValueType
cos_indirect_obj_node_get_type(const CosIndirectObjNode *indirect_obj)
    COS_WARN_UNUSED_RESULT;

void
cos_indirect_obj_node_print_desc(const CosIndirectObjNode *indirect_obj);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_INDIRECT_OBJ_H */
