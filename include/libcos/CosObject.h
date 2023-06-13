//
// Created by david on 18/05/23.
//

#ifndef LIBCOS_COS_OBJECT_H
#define LIBCOS_COS_OBJECT_H

#include <libcos/CosDocument.h>

struct CosDocument;

struct CosObject;
typedef struct CosObject CosObject;

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

struct CosDocument *
cos_object_get_document(CosObject *obj);

typedef struct CosArrayObject CosArrayObject;

unsigned int
cos_array_get_len(CosArrayObject *obj);

#endif //LIBCOS_COS_OBJECT_H
