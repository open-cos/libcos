/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_OBJECTS_COS_REFERENCE_OBJ_NODE_H
#define LIBCOS_OBJECTS_COS_REFERENCE_OBJ_NODE_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>
#include <libcos/objects/CosObjNodeTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

CosReferenceObjNode * COS_Nullable
cos_reference_obj_node_alloc(CosObjID id,
                        CosDoc *document)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_reference_obj_node_free(CosReferenceObjNode *reference_obj);

CosObjNodeValueType
cos_reference_obj_node_get_type(CosReferenceObjNode *reference_obj)
    COS_WARN_UNUSED_RESULT;

CosObjNode * COS_Nullable
cos_reference_obj_node_get_value(CosReferenceObjNode *reference_obj)
    COS_WARN_UNUSED_RESULT;

void
cos_reference_obj_node_print_desc(const CosReferenceObjNode *reference_obj);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_OBJECTS_COS_REFERENCE_OBJ_NODE_H */
