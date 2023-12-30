//
// Created by david on 15/10/23.
//

#include "libcos/CosArrayObj.h"

#include "CosArrayObj-Impl.h"
#include "CosObj-Impl.h"
#include "common/Assert.h"
#include "libcos/common/CosClass.h"
#include "libcos/common/CosObj.h"
#include "libcos/private/common/CosClass-Impl.h"
#include "libcos/private/common/CosObj-Impl.h"

#include <libcos/common/CosArray.h>

#include <stddef.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

static CosClass cos_array_obj_class_;

static void
cos_array_obj_init_impl_(CosArrayObj *obj);

static void
cos_array_obj_dealloc_impl_(CosObj *obj);

CosClass *
cos_array_obj_class(void)
{
    cos_class_init(&cos_array_obj_class_,
                   cos_base_obj_class(),
                   NULL,
                   &cos_array_obj_dealloc_impl_);

    return &cos_array_obj_class_;
}

static void
cos_array_retain_obj_(void *item)
{
    CosObj * const obj = (CosObj *)item;

    cos_obj_retain(obj);
}

static void
cos_array_release_obj_(void *item)
{
    CosObj * const obj = (CosObj *)item;

    cos_obj_release(obj);
}

static void
cos_array_obj_init_impl_(CosArrayObj *obj)
{
    ((CosObj *)obj)->class->init((CosObj *)obj);

    CosArray * const array = cos_array_alloc((CosArrayCallbacks){
        .retain = &cos_array_retain_obj_,
        .release = &cos_array_release_obj_,
        .equal = NULL,
    });
    if (!array) {
        return;
    }

    obj->array_value = array;
}

static void
cos_array_obj_dealloc_impl_(CosObj *obj)
{
    CosArrayObj * const array_obj = (CosArrayObj *)obj;

    cos_array_free(array_obj->array_value);

    obj->class->super->dealloc(obj);
}

CosArrayObj *
cos_array_obj_create(void)
{
    CosArrayObj * const obj = cos_obj_alloc(sizeof(CosArrayObj),
                                            CosObjectType_Array,
                                            NULL);
    if (!obj) {
        return NULL;
    }

    obj->objects = NULL;
    obj->count = 0;

    return obj;
}

bool
cos_array_object_append(CosArrayObj *array_obj,
                        CosBaseObj *object,
                        CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(array_obj != NULL);
    COS_PARAMETER_ASSERT(object != NULL);

    if (!array_obj) {
        return false;
    }

    void * const objects = realloc(array_obj->objects,
                                   sizeof(CosBaseObj *) * (array_obj->count + 1));
    if (!objects) {
        if (error) {
            COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                               "Failed to allocate memory for array object"),
                                error);
        }
        return false;
    }

    array_obj->objects = objects;
    array_obj->objects[array_obj->count] = object;
    array_obj->count++;

    return true;
}

COS_ASSUME_NONNULL_END
