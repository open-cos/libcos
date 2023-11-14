//
// Created by david on 16/10/23.
//

#include "libcos/CosStringObj.h"

#include "CosStringObj-Impl.h"
#include "common/Assert.h"

#include <libcos/CosObj.h>

#include <stdlib.h>
#include <string.h>

CosStringObj * COS_Nullable
cos_string_obj_alloc(unsigned char *value,
                     size_t length)
{
    CosStringObj * const obj = cos_obj_alloc(sizeof(CosStringObj),
                                             CosObjectType_String,
                                             NULL);

    if (!obj) {
        return NULL;
    }

    obj->value = NULL;
    obj->length = 0;

    return obj;
}

bool
cos_string_obj_get_value(const CosStringObj *string_obj,
                         const unsigned char * COS_Nullable *value,
                         size_t *length,
                         CosError ** COS_Nullable error)
{
    COS_PARAMETER_ASSERT(string_obj != NULL);
    COS_PARAMETER_ASSERT(value != NULL);
    COS_PARAMETER_ASSERT(length != NULL);

    if (!string_obj) {
        cos_error_propagate(cos_error_alloc(COS_ERROR_INVALID_ARGUMENT,
                                            "string_obj is NULL"),
                            error);
        return false;
    }

    if (value) {
        *value = string_obj->value;
    }

    if (length) {
        *length = string_obj->length;
    }

    return true;
}

bool
cos_string_obj_set_value(CosStringObj *string_obj,
                         const unsigned char *value,
                         size_t length,
                         CosError ** COS_Nullable error)
{
    COS_PARAMETER_ASSERT(string_obj != NULL);
    COS_PARAMETER_ASSERT(value != NULL);

    if (!string_obj) {
        cos_error_propagate(cos_error_alloc(COS_ERROR_INVALID_ARGUMENT,
                                            "string_obj is NULL"),
                            error);
        return false;
    }

    if (!value) {
        cos_error_propagate(cos_error_alloc(COS_ERROR_INVALID_ARGUMENT,
                                            "value is NULL"),
                            error);
        return false;
    }

    void * const new_value = realloc(string_obj->value,
                                     length);
    if (!new_value) {
        cos_error_propagate(cos_error_alloc(COS_ERROR_MEMORY,
                                            "Failed to allocate memory for string object"),
                            error);
        return false;
    }

    string_obj->value = new_value;

    memcpy(new_value,
           value,
           length);

    string_obj->length = length;

    return true;
}
