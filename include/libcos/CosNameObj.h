//
// Created by david on 15/10/23.
//

#ifndef LIBCOS_COS_NAME_OBJ_H
#define LIBCOS_COS_NAME_OBJ_H

#include <libcos/CosBaseObj.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosTypes.h>
#include <libcos/private/objects/CosBaseObj-Impl.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

CosNameObj * COS_Nullable
cos_name_obj_create(CosString *value)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

bool
cos_name_obj_set_value(CosNameObj *name_obj,
                       CosString *value,
                       CosError * COS_Nullable error);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_NAME_OBJ_H */
