/*
 * Copyright (c) 2023-2024 OpenCOS.
 */

#ifndef LIBCOS_COS_REAL_OBJ_H
#define LIBCOS_COS_REAL_OBJ_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

COS_API CosRealObjNode * COS_Nullable
cos_real_obj_node_alloc(double value)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

COS_API void
cos_real_obj_node_free(CosRealObjNode *real_obj);

// MARK: - Value accessors

COS_API double
cos_real_obj_node_get_value(const CosRealObjNode *real_obj);

COS_API void
cos_real_obj_node_set_value(CosRealObjNode *real_obj,
                       double value);

COS_API void
cos_real_obj_node_print_desc(const CosRealObjNode *real_obj);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_REAL_OBJ_H */
