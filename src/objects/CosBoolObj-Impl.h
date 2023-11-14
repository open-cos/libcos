//
// Created by david on 15/10/23.
//

#ifndef LIBCOS_COS_BOOL_OBJ_IMPL_H
#define LIBCOS_COS_BOOL_OBJ_IMPL_H

#include "CosObj-Impl.h"

#include <stdbool.h>

struct CosBoolObj {
    CosObj base;

    bool value;
};

#endif /* LIBCOS_COS_BOOL_OBJ_IMPL_H */
