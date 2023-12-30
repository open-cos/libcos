/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_CLASS_H
#define LIBCOS_COMMON_COS_CLASS_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

void
cos_class_init(CosClass *class,
               CosClass * COS_Nullable super,
               void (* COS_Nullable init)(CosObj *self),
               void (* COS_Nullable dealloc)(CosObj *self));

void * COS_Nullable
cos_class_alloc_obj(CosClass *class,
                    size_t size);

#pragma mark - Root Class

/**
 * @brief Returns the root class of all objects.
 *
 * @return The root class.
 */
CosClass *
cos_obj_root_class(void);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_CLASS_H */
