//
// Created by david on 18/05/23.
//

#include "libcos/CosObject.h"

#include "libcos/common/CosError.h"

#include <stdbool.h>
#include <stdlib.h>

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

CosObject *
cos_array_object_alloc(void)
{
    CosObject * const obj = malloc(sizeof(CosObject));
    if (!obj) {
        return NULL;
    }

    obj->type = CosObjectType_Array;
    obj->document = NULL;

    return obj;
}

bool
cos_array_object_append(CosObject *array_object,
                        CosObject *object,
                        CosError **error)
{
    return false;
}
