//
// Created by david on 18/05/23.
//

#ifndef LIBCOS_OBJECTS_COS_BASE_OBJ_H
#define LIBCOS_OBJECTS_COS_BASE_OBJ_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef enum CosObjectType {
    CosObjectType_Unknown = 0,

    CosObjectType_Boolean,
    CosObjectType_String,
    CosObjectType_Name,
    CosObjectType_Integer,
    CosObjectType_Real,
    CosObjectType_Array,
    CosObjectType_Dictionary,
    CosObjectType_Stream,
    CosObjectType_Null,
} CosObjectType;

void * COS_Nullable
cos_obj_alloc(size_t size,
              CosObjectType type,
              CosDoc * COS_Nullable document)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

CosDoc * COS_Nullable
cos_obj_get_document(const CosBaseObj *obj);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_OBJECTS_COS_BASE_OBJ_H */
