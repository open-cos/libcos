//
// Created by david on 15/10/23.
//

#include "libcos/objects/CosNameObj.h"

#include "common/Assert.h"
#include "libcos/common/CosString.h"
#include "libcos/objects/CosObj.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosNameObj {
    CosObjType type;

    CosString *value;
};

CosNameObj *
cos_name_obj_alloc(CosString *value)
{
    COS_PARAMETER_ASSERT(value != NULL);
    if (!value) {
        return NULL;
    }

    CosNameObj * const name_obj = calloc(1, sizeof(CosNameObj));
    if (!name_obj) {
        return NULL;
    }

    name_obj->type = CosObjType_Name;
    name_obj->value = value;

    return name_obj;
}

CosString *
cos_name_obj_get_value(const CosNameObj *name_obj)
{
    COS_PARAMETER_ASSERT(name_obj != NULL);
    if (!name_obj) {
        return NULL;
    }

    return name_obj->value;
}

void
cos_name_obj_set_value(CosNameObj *name_obj,
                       CosString *value)
{
    COS_PARAMETER_ASSERT(name_obj != NULL);
    COS_PARAMETER_ASSERT(value != NULL);
    if (!name_obj || !value) {
        return;
    }

    if (name_obj->value == value) {
        // No change.
        return;
    }

    // Free the old value.
    cos_string_free(name_obj->value);

    name_obj->value = value;
}

void
cos_name_obj_free(CosNameObj *name_obj)
{
    if (!name_obj) {
        return;
    }

    cos_string_free(name_obj->value);
    free(name_obj);
}

COS_ASSUME_NONNULL_END
