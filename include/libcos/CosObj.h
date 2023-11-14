//
// Created by david on 18/05/23.
//

#ifndef LIBCOS_COS_OBJECT_H
#define LIBCOS_COS_OBJECT_H

#include <libcos/CosDocument.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef enum CosObjectType {
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
              CosDocument * COS_Nullable document)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_obj_free(CosObj *obj);

CosObjectType
cos_obj_get_type(CosObj *obj);

CosDocument *
cos_obj_get_document(CosObj *obj);

CosDictObj *
cos_dict_obj_alloc(void)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif //LIBCOS_COS_OBJECT_H
