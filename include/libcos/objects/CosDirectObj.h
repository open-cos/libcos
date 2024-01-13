/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_OBJECTS_COS_DIRECT_OBJ_H
#define LIBCOS_OBJECTS_COS_DIRECT_OBJ_H

#include <libcos/CosObject.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief Returns the type of a direct object.
 *
 * @param direct_obj The direct object.
 *
 * @return The type of the direct object.
 */
CosObjectType
cos_direct_obj_get_type(const CosDirectObj *direct_obj);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_OBJECTS_COS_DIRECT_OBJ_H */
