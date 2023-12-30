//
// Created by david on 15/10/23.
//

#ifndef LIBCOS_COS_ARRAY_OBJ_IMPL_H
#define LIBCOS_COS_ARRAY_OBJ_IMPL_H

#include <libcos/private/objects/CosBaseObj-Impl.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef struct CosArray CosArray;

struct CosArrayObj {
    CosBaseObj base;

    CosBaseObj **objects;
    size_t count;

    CosArray *array_value;
};

CosClass *
cos_array_obj_class(void);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_ARRAY_OBJ_IMPL_H */
