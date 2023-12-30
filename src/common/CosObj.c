/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/common/CosObj.h"

#include "common/Assert.h"
#include "libcos/private/common/CosClass-Impl.h"
#include "libcos/private/common/CosObj-Impl.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

#pragma mark - Reference Counting

void
cos_obj_retain(CosObj *obj)
{
    COS_PARAMETER_ASSERT(obj != NULL);
    if (!obj) {
        return;
    }

    // Only objects with a positive reference count can be retained.
    if (obj->ref_count > 0) {
        obj->ref_count++;
    }
}

void
cos_obj_release(CosObj *obj)
{
    COS_PARAMETER_ASSERT(obj != NULL);
    if (!obj) {
        return;
    }

    // Avoid over-releasing objects.
    if (obj->ref_count > 0) {
        obj->ref_count--;

        // The object is deallocated when the reference count reaches zero.
        if (obj->ref_count == 0) {
            obj->class->dealloc(obj);
        }
    }
}

COS_ASSUME_NONNULL_END
