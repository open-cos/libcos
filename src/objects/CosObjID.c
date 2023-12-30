/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/CosObjID.h"

COS_ASSUME_NONNULL_BEGIN

CosObjID
cos_obj_id_make(unsigned int object_number,
                unsigned int generation_number)
{
    CosObjID obj_id;
    obj_id.object_number = object_number;
    obj_id.generation_number = generation_number;
    return obj_id;
}

int
cos_obj_id_compare(CosObjID lhs, CosObjID rhs)
{
    if (lhs.generation_number < rhs.generation_number) {
        return -1;
    }
    else if (lhs.generation_number > rhs.generation_number) {
        return 1;
    }
    // The generation numbers are equal; compare the object numbers.
    if (lhs.object_number < rhs.object_number) {
        return -1;
    }
    else if (lhs.object_number > rhs.object_number) {
        return 1;
    }

    // The object IDs are equal.
    return 0;
}

COS_ASSUME_NONNULL_END
