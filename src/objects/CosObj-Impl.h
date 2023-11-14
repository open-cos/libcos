//
// Created by david on 15/10/23.
//

#ifndef LIBCOS_COS_OBJ_IMPL_H
#define LIBCOS_COS_OBJ_IMPL_H

#include "libcos/CosObj.h"

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef struct CosObjClass CosObjClass;

struct CosObjClass {
    CosObjClass * COS_Nullable super_class;

    void (* COS_Nullable free)(CosObj *obj);
};

const CosObjClass cos_obj_class;

struct CosObj {
    CosObjClass *class;

    CosObjectType type;
    CosDocument * COS_Nullable document;

    CosObjRef * COS_Nullable reference;
};

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_OBJ_IMPL_H */
