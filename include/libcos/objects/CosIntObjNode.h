/*
 * Copyright (c) 2023-2024 OpenCOS.
 */

#ifndef LIBCOS_COS_NUMBER_OBJ_H
#define LIBCOS_COS_NUMBER_OBJ_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

CosIntObjNode * COS_Nullable
cos_int_obj_node_alloc(int value)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_int_obj_node_free(CosIntObjNode *int_obj);

// MARK: - Value accessors

int
cos_int_obj_node_get_value(const CosIntObjNode *int_obj);

void
cos_int_obj_node_set_value(CosIntObjNode *int_obj,
                      int value);

void
cos_int_obj_node_print_desc(const CosIntObjNode *int_obj);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_NUMBER_OBJ_H */
