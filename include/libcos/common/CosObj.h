/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_OBJ_H
#define LIBCOS_COMMON_COS_OBJ_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

#pragma mark - Reference Counting

/**
 * @brief Retains the object.
 *
 * This increases the object's reference count by one.
 *
 * @param obj The object.
 */
void
cos_obj_retain(CosObj *obj);

/**
 * @brief Releases the object.
 *
 * This decreases the object's reference count by one.
 * If the object's reference count reaches zero, the object is deallocated.
 *
 * @param obj The object.
 */
void
cos_obj_release(CosObj *obj);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_OBJ_H */
