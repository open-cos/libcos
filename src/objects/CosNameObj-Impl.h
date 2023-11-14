//
// Created by david on 15/10/23.
//

#ifndef LIBCOS_COS_NAME_OBJ_IMPL_H
#define LIBCOS_COS_NAME_OBJ_IMPL_H

#include <libcos/CosNameObj.h>

#include "CosObj-Impl.h"

const CosObjClass cos_name_obj_class;

struct CosNameObj {
    CosObj base;

    unsigned char *value;
    size_t length;
};

#endif /* LIBCOS_COS_NAME_OBJ_IMPL_H */
