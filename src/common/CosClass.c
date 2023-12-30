/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/common/CosClass.h"

#include "common/Assert.h"
#include "libcos/private/common/CosClass-Impl.h"
#include "libcos/private/common/CosObj-Impl.h"

#include <stdbool.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

static CosClass cos_obj_root_class_;

static void
cos_obj_root_init_(CosObj *obj)
{
    obj->ref_count = 1;
}

static void
cos_obj_root_dealloc_(CosObj *obj)
{
    free(obj);
}

void
cos_class_init(CosClass *class,
               CosClass * COS_Nullable super,
               void (* COS_Nullable init)(CosObj *self),
               void (* COS_Nullable dealloc)(CosObj *self))
{
    COS_PARAMETER_ASSERT(class != NULL);
    if (!class) {
        return;
    }

    class->super = super;

    class->init = init;
    class->dealloc = dealloc;
}

void *
cos_class_alloc_obj(CosClass *class,
                    size_t size)
{
    COS_PARAMETER_ASSERT(class != NULL);
    if (!class) {
        return NULL;
    }

    CosObj * const obj = calloc(1, size);
    if (!obj) {
        return NULL;
    }

    obj->class = class;

    return obj;
}

#pragma mark - Root Class

CosClass *
cos_obj_root_class(void)
{
    static volatile bool initialized = false;
    if (!initialized) {
        initialized = true;

        cos_class_init(&cos_obj_root_class_,
                       NULL,
                       &cos_obj_root_init_,
                       &cos_obj_root_dealloc_);
    }
    return &cos_obj_root_class_;
}

COS_ASSUME_NONNULL_END
