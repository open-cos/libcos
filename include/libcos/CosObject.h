//
// Created by david on 18/05/23.
//

#ifndef LIBCOS_COS_OBJECT_H
#define LIBCOS_COS_OBJECT_H

#include <libcos/CosDocument.h>

typedef struct CosDocument CosDocument;
typedef struct CosObject CosObject;

typedef enum CosObjectType {
    CosObjectType_Boolean,
    CosObjectType_Array,
    CosObjectType_Dictionary,
    CosObjectType_String,
} CosObjectType;

CosObjectType
cos_object_get_type(CosObject *obj);

CosDocument *
cos_object_get_document(CosObject *obj);

typedef struct CosArrayObject CosArrayObject;

unsigned int
cos_array_get_len(CosArrayObject *obj);

#endif //LIBCOS_COS_OBJECT_H
