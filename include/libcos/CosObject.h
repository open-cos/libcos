//
// Created by david on 18/05/23.
//

#ifndef LIBCOS_COS_OBJECT_H
#define LIBCOS_COS_OBJECT_H

#include "libcos/common/CosError.h"
#include <libcos/CosDocument.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>

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

CosObjectType
cos_object_get_type(CosObject *obj);

CosDocument *
cos_object_get_document(CosObject *obj);

CosObject *
cos_array_object_alloc(void);

bool
cos_array_object_append(CosObject *array_object,
                        CosObject *object,
                        CosError **error);

#endif //LIBCOS_COS_OBJECT_H
