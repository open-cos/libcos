/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_OBJECTS_COS_REFERENCE_OBJ_NODE_H
#define LIBCOS_OBJECTS_COS_REFERENCE_OBJ_NODE_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>
#include <libcos/objects/CosObjNodeTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

COS_API CosReferenceObjNode * COS_Nullable
cos_reference_obj_node_alloc(CosObjID id,
                        CosDoc *document)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

COS_API void
cos_reference_obj_node_free(CosReferenceObjNode *reference_obj);

/**
 * Gets the identifier of the object that @p reference_obj refers to.
 *
 * Unlike @c cos_reference_obj_node_get_value , this does not resolve the reference: it
 * reports the referenced object's identifier without loading the object itself.
 *
 * @param reference_obj The indirect reference.
 *
 * @return The referenced object's identifier, or @c CosObjID_Invalid on error.
 */
COS_API CosObjID
cos_reference_obj_node_get_id(const CosReferenceObjNode *reference_obj)
    COS_ATTR_PURE
    COS_WARN_UNUSED_RESULT;

COS_API CosObjNodeValueType
cos_reference_obj_node_get_type(CosReferenceObjNode *reference_obj)
    COS_WARN_UNUSED_RESULT;

COS_API CosObjNode * COS_Nullable
cos_reference_obj_node_get_value(CosReferenceObjNode *reference_obj)
    COS_WARN_UNUSED_RESULT;

COS_API void
cos_reference_obj_node_print_desc(const CosReferenceObjNode *reference_obj);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_OBJECTS_COS_REFERENCE_OBJ_NODE_H */
