//
// Created by david on 15/10/23.
//

#include "libcos/objects/CosNameObj.h"

#include "common/Assert.h"

#include "libcos/common/CosString.h"
#include "libcos/objects/CosObj.h"

#include <stdio.h>
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

void
cos_name_obj_free(CosNameObj *name_obj)
{
    if (!name_obj) {
        return;
    }

    cos_string_free(name_obj->value);
    free(name_obj);
}

const CosString *
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

size_t
cos_name_obj_get_hash(const CosNameObj *name_obj)
{
    COS_PARAMETER_ASSERT(name_obj != NULL);
    if (!name_obj) {
        return 0;
    }

    return cos_string_get_hash(name_obj->value);
}

bool
cos_name_obj_equal(const CosNameObj *name_obj1,
                   const CosNameObj *name_obj2)
{
    COS_PARAMETER_ASSERT(name_obj1 != NULL);
    COS_PARAMETER_ASSERT(name_obj2 != NULL);
    if (!name_obj1 || !name_obj2) {
        return false;
    }

    return cos_string_ref_cmp(cos_string_get_ref(name_obj1->value),
                              cos_string_get_ref(name_obj2->value)) == 0;
}

void
cos_name_obj_print_desc(const CosNameObj *name_obj)
{
    COS_PARAMETER_ASSERT(name_obj != NULL);
    if (!name_obj) {
        return;
    }

    printf("Name: %s\n", cos_string_get_data(name_obj->value));
}

COS_ASSUME_NONNULL_END
