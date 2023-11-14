//
// Created by david on 15/10/23.
//

#ifndef LIBCOS_COS_ARRAY_OBJ_IMPL_H
#define LIBCOS_COS_ARRAY_OBJ_IMPL_H

#include "CosObj-Impl.h"

struct CosArrayObj {
    CosObj base;

    CosObj **objects;
    size_t count;
};

void
cos_array_obj_free_impl(CosArrayObj *array_obj);

#endif /* LIBCOS_COS_ARRAY_OBJ_IMPL_H */
