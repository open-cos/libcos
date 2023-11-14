//
// Created by david on 15/10/23.
//

#include "libcos/CosNameObj.h"

#include "CosNameObj-Impl.h"
#include "common/Assert.h"
#include "libcos/CosObj.h"

#include <stdlib.h>
#include <string.h>

CosNameObj * COS_Nullable
cos_name_obj_alloc(void)
{
    CosNameObj * const obj = cos_obj_alloc(sizeof(CosNameObj),
                                           CosObjectType_Name,
                                           NULL);

    if (!obj) {
        return NULL;
    }

    obj->value = NULL;
    obj->length = 0;

    return obj;
}

bool
cos_name_obj_get_value(const CosNameObj *name_obj,
                       const unsigned char ** COS_Nullable value,
                       size_t * COS_Nullable length,
                       CosError ** COS_Nullable error)
{
    COS_PARAMETER_ASSERT(name_obj != NULL);

    if (!name_obj) {
        if (error) {
            *error = cos_error_alloc(COS_ERROR_INVALID_ARGUMENT,
                                     "Name object is NULL");
        }
        return false;
    }

    if (value) {
        *value = name_obj->value;
    }
    if (length) {
        *length = name_obj->length;
    }

    return true;
}

bool
cos_name_obj_set_value(CosNameObj *name_obj,
                       const unsigned char *value,
                       size_t length,
                       CosError ** COS_Nullable error)
{
    COS_PARAMETER_ASSERT(name_obj != NULL);

    if (!name_obj) {
        if (error) {
            *error = cos_error_alloc(COS_ERROR_INVALID_ARGUMENT,
                                     "Name object is NULL");
        }
        return false;
    }

    void * const new_value = realloc(name_obj->value,
                                     length);
    if (!new_value) {
        if (error) {
            *error = cos_error_alloc(COS_ERROR_MEMORY,
                                     "Failed to allocate memory for name object");
        }
        return false;
    }

    name_obj->value = new_value;

    memcpy(new_value,
           value,
           length);

    name_obj->length = length;

    return true;
}
