/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/CosObjID.h"

COS_ASSUME_NONNULL_BEGIN

const CosObjID CosObjID_Invalid = {0};

CosObjID
cos_obj_id_make(unsigned int obj_number,
                unsigned int gen_number)
{
    CosObjID obj_id;
    obj_id.obj_number = obj_number;
    obj_id.gen_number = gen_number;
    return obj_id;
}

bool
cos_obj_id_is_valid(CosObjID obj_id)
{
    return obj_id.obj_number > 0;
}

int
cos_obj_id_compare(CosObjID lhs, CosObjID rhs)
{
    if (lhs.gen_number < rhs.gen_number) {
        return -1;
    }
    else if (lhs.gen_number > rhs.gen_number) {
        return 1;
    }
    // The generation numbers are equal; compare the object numbers.
    if (lhs.obj_number < rhs.obj_number) {
        return -1;
    }
    else if (lhs.obj_number > rhs.obj_number) {
        return 1;
    }

    // The object IDs are equal.
    return 0;
}

COS_ASSUME_NONNULL_END
