//
// Created by david on 23/09/23.
//

#ifndef LIBCOS_COS_OBJ_REF_H
#define LIBCOS_COS_OBJ_REF_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

struct CosObjRef {
    struct {
        unsigned int obj_num;
        unsigned int gen_num;
    } id;

    CosBaseObj * COS_Nullable obj;

    unsigned long ref_count;
};

#endif /* LIBCOS_COS_OBJ_REF_H */
