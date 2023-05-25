//
// Created by david on 18/05/23.
//

#include "libcos/CosObject.h"

struct CosObject {
    CosObjectType type;
    CosDocument *document;
};

CosObjectType
cos_object_get_type(CosObject *obj)
{
    return obj->type;
}

CosDocument *
cos_object_get_document(CosObject *obj)
{
    return obj->document;
}



struct CosArrayObject {
    CosObject base;

    char *elements;
};

unsigned int
cos_array_get_len(CosArrayObject *obj)
{
    return 0;
}
