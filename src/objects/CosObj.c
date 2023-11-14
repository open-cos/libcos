//
// Created by david on 18/05/23.
//

#include "libcos/CosObj.h"

#include "common/Assert.h"
#include "objects/CosArrayObj-Impl.h"
#include "objects/CosObj-Impl.h"

#include <stdlib.h>

const CosObjClass cos_obj_class = {
    .super_class = NULL,
    .free = cos_obj_free,
};

struct CosDictObj {
    CosObj base;

    CosObj **keys;
    CosObj **values;
    size_t count;
};

void *
cos_obj_alloc(size_t size,
              CosObjectType type,
              CosDocument *document)
{
    CosObj * const object = malloc(size);
    if (!object) {
        return NULL;
    }

    object->type = type;
    object->document = document;
    object->reference = NULL;

    return object;
}

static void
cos_obj_free_impl(CosObj *obj)
{
    free(obj);
}

void
cos_obj_free(CosObj *obj)
{
    if (!obj) {
        return;
    }

    switch (obj->type) {
        case CosObjectType_Dictionary: {
            break;
        }
        case CosObjectType_Array: {
            cos_array_obj_free_impl((CosArrayObj *)obj);
            break;
        }
        case CosObjectType_Name: {
            break;
        }
        case CosObjectType_String: {
            break;
        }
        case CosObjectType_Boolean:
        case CosObjectType_Null:
            break;
    }

    cos_obj_free_impl(obj);
}

CosObjectType
cos_obj_get_type(CosObj *obj)
{
    return obj->type;
}

CosDocument *
cos_obj_get_document(CosObj *obj)
{
    return obj->document;
}

CosDictObj *
cos_dict_obj_alloc(void)
{
    CosDictObj * const obj = cos_obj_alloc(sizeof(CosDictObj),
                                           CosObjectType_Dictionary,
                                           NULL);
    if (!obj) {
        return NULL;
    }

    obj->keys = NULL;
    obj->values = NULL;
    obj->count = 0;

    return obj;
}
