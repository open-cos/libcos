//
// Created by david on 16/10/23.
//

#ifndef LIBCOS_COS_STRING_OBJ_H
#define LIBCOS_COS_STRING_OBJ_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

CosStringObj * COS_Nullable
cos_string_obj_alloc(unsigned char * value,
                     size_t length)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

bool
cos_string_obj_get_value(const CosStringObj *string_obj,
                         const unsigned char * COS_Nullable *value,
                         size_t *length,
                         CosError ** COS_Nullable error);

bool
cos_string_obj_set_value(CosStringObj *string_obj,
                         const unsigned char *value,
                         size_t length,
                         CosError ** COS_Nullable error);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_STRING_OBJ_H */
