//
// Created by david on 16/10/23.
//

#ifndef LIBCOS_COS_STRING_OBJ_H
#define LIBCOS_COS_STRING_OBJ_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosTypes.h>
#include <libcos/private/objects/CosBaseObj-Impl.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

CosStringObj * COS_Nullable
cos_string_obj_alloc(CosData *string_data,
                     CosDoc * COS_Nullable document)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_STRING_OBJ_H */
