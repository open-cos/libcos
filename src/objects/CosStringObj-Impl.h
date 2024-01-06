//
// Created by david on 16/10/23.
//

#ifndef LIBCOS_COS_STRING_OBJ_IMPL_H
#define LIBCOS_COS_STRING_OBJ_IMPL_H

#include "libcos/CosBaseObj.h"
#include "libcos/CosStringObj.h"
#include "libcos/common/CosData.h"

struct CosStringObj {
    CosBaseObj base;

    CosData *data;
};

#endif /* LIBCOS_COS_STRING_OBJ_IMPL_H */
