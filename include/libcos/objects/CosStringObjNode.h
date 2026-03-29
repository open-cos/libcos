/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COS_STRING_OBJ_H
#define LIBCOS_COS_STRING_OBJ_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

void
cos_string_obj_node_free(CosStringObjNode *string_obj)
    COS_DEALLOCATOR_FUNC;

CosStringObjNode * COS_Nullable
cos_string_obj_node_alloc(CosData *data)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_string_obj_node_free)
    COS_OWNERSHIP_HOLDS(1);

const CosData * COS_Nullable
cos_string_obj_node_get_value(const CosStringObjNode *string_obj);

void
cos_string_obj_node_set_value(CosStringObjNode *string_obj,
                         CosData *data);

void
cos_string_obj_node_print_desc(const CosStringObjNode *string_obj);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_STRING_OBJ_H */
