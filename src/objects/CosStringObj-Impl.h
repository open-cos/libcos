//
// Created by david on 16/10/23.
//

#ifndef LIBCOS_COS_STRING_OBJ_IMPL_H
#define LIBCOS_COS_STRING_OBJ_IMPL_H

#include "CosObj-Impl.h"
#include "libcos/CosStringObj.h"

struct CosStringObj {
    CosObj base;

    unsigned char *value;
    size_t length;
};

#endif /* LIBCOS_COS_STRING_OBJ_IMPL_H */
