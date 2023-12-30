//
// Created by david on 15/10/23.
//

#include "libcos/CosNameObj.h"

#include "common/Assert.h"
#include "common/CosString.h"
#include "libcos/common/CosClass.h"
#include "libcos/private/objects/CosNameObj-Impl.h"

COS_ASSUME_NONNULL_BEGIN

static CosNameObjClass cos_name_obj_class_;

struct CosClassInitializer {
    CosNameObjClass *class;
    bool initialized;
};

static struct CosClassInitializer cos_name_obj_class_initializer_ = {
    .class = &cos_name_obj_class_,
    .initialized = false,
};

static void
cos_name_obj_class_init_(CosNameObjClass *class);

static void
cos_name_obj_init_impl_(CosNameObj *obj,
                        CosString *value);

static void
cos_name_obj_dealloc_impl_(CosObj *obj);

#pragma mark - Private Implementations

static void
cos_name_obj_class_init_(CosNameObjClass *class)
{
    cos_class_init(&(class->base),
                   cos_base_obj_class(),
                   NULL,
                   &cos_name_obj_dealloc_impl_);

    class->init = &cos_name_obj_init_impl_;
}

static void
cos_name_obj_init_impl_(CosNameObj *name_obj,
                        CosString *value)
{
    ((CosObj *)name_obj)->class->init((CosObj *)name_obj);

    name_obj->value = value;
}

static void
cos_name_obj_dealloc_impl_(CosObj *obj)
{
    CosNameObj * const name_obj = (CosNameObj *)obj;

    // Free the name's string value.
    cos_string_free(name_obj->value);

    obj->class->super->dealloc(obj);
}

CosClass *
cos_name_obj_class(void)
{
    if (!cos_name_obj_class_initializer_.initialized) {
        cos_name_obj_class_initializer_.initialized = true;

        cos_name_obj_class_init_(cos_name_obj_class_initializer_.class);
    }

    return (CosClass *)cos_name_obj_class_initializer_.class;
}

#pragma mark - Public

CosNameObj *
cos_name_obj_create(CosString *value)
{
    COS_PARAMETER_ASSERT(value != NULL);

    CosNameObj * const name_obj = cos_class_alloc_obj(cos_name_obj_class(),
                                                      sizeof(CosNameObj));
    if (!name_obj) {
        return NULL;
    }

    ((CosNameObjClass *)name_obj->base.base.class)->init(name_obj, value);

    return name_obj;
}

bool
cos_name_obj_set_value(CosNameObj *name_obj,
                       CosString *value,
                       CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(name_obj != NULL);
    COS_PARAMETER_ASSERT(value != NULL);

    if (!name_obj) {
        if (error) {
            *error = cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                    "Name object is NULL");
        }
        return false;
    }

    if (value) {
        name_obj->value = value;
    }

    return true;
}

COS_ASSUME_NONNULL_END
