/*
 * Copyright (c) 2023-2024 OpenCOS.
 */

#ifndef LIBCOS_COS_NULL_OBJ_H
#define LIBCOS_COS_NULL_OBJ_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief Returns the null object.
 *
 * There is only one null object.
 *
 * @return The null object.
 */
CosNullObj *
cos_null_obj_get(void);

void
cos_null_obj_print_desc(const CosNullObj *null_obj);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_NULL_OBJ_H */
