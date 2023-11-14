//
// Created by david on 15/10/23.
//

#include "libcos/CosArrayObj.h"

#include "CosArrayObj-Impl.h"
#include "CosObj-Impl.h"
#include "common/Assert.h"

#include <stddef.h>
#include <stdlib.h>

CosArrayObj *
cos_array_obj_alloc(void)
{
    CosArrayObj * const obj = cos_obj_alloc(sizeof(CosArrayObj),
                                            CosObjectType_Array,
                                            NULL);
    if (!obj) {
        return NULL;
    }

    obj->objects = NULL;
    obj->count = 0;

    return obj;
}

void
cos_array_obj_free_impl(CosArrayObj *array_obj)
{
    if (!array_obj) {
        return;
    }

    for (size_t i = 0; i < array_obj->count; i++) {
        cos_obj_free(array_obj->objects[i]);
    }

    free(array_obj->objects);
    free(array_obj);
}

bool
cos_array_object_append(CosArrayObj *array_obj,
                        CosObj *object,
                        CosError ** COS_Nullable error)
{
    COS_PARAMETER_ASSERT(array_obj != NULL);
    COS_PARAMETER_ASSERT(object != NULL);

    if (!array_obj) {
        return false;
    }

    void * const objects = realloc(array_obj->objects,
                                   sizeof(CosObj *) * (array_obj->count + 1));
    if (!objects) {
        if (error) {
            *error = cos_error_alloc(COS_ERROR_MEMORY,
                                     "Failed to allocate memory for array object");
        }
        return false;
    }

    array_obj->objects = objects;
    array_obj->objects[array_obj->count] = object;
    array_obj->count++;

    return true;
}
