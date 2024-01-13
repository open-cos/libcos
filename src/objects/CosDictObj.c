/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/objects/CosDictObj.h"

#include "common/Assert.h"
#include "libcos/common/CosClass.h"
#include "libcos/common/CosDict.h"
#include "libcos/private/objects/CosDictObj-Impl.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

static CosDictObjClass cos_dict_obj_class_;

struct CosClassInitializer {
    CosDictObjClass *class;
    bool initialized;
};

static void
cos_dict_obj_class_init_(CosDictObjClass *class);

static void
cos_dict_obj_init_(CosDictObj *dict_obj,
                   CosDict *dict);

static void
cos_dict_obj_dealloc_(CosObj *obj);

static struct CosClassInitializer cos_dict_obj_class_initializer_ = {
    .class = &cos_dict_obj_class_,
    .initialized = false,
};

#pragma mark - Private Implementations

static void
cos_dict_obj_class_init_(CosDictObjClass *class)
{
    cos_class_init((CosClass *)class,
                   cos_base_obj_class(),
                   NULL,
                   &cos_dict_obj_dealloc_);

    class->base.get_type = NULL;

    class->init = &cos_dict_obj_init_;
}

static void
cos_dict_obj_init_(CosDictObj *dict_obj,
                   CosDict *dict)
{
    ((CosObj *)dict_obj)->class->init((CosObj *)dict_obj);

    dict_obj->dict = dict;
}

static void
cos_dict_obj_dealloc_(CosObj *obj)
{
    CosDictObj * const dict_obj = (CosDictObj *)obj;

    free(dict_obj->dict);

    ((CosObj *)dict_obj)->class->super->dealloc(obj);
}

CosDictObj *
cos_dict_obj_create(void)
{
    CosDictObj * const dict_obj = cos_class_alloc_obj(cos_dict_obj_class(),
                                                      sizeof(CosDictObj));
    if (!dict_obj) {
        goto failure;
    }

    CosDict * const dict = cos_dict_alloc(0,
                                          NULL,
                                          NULL);
    if (!dict) {
        goto failure;
    }

    ((CosDictObjClass *)dict_obj->base.base.class)->init(dict_obj, dict);

    return dict_obj;

failure:
    if (dict_obj) {
        // TODO: Deallocate the obj directly?
        cos_obj_release((CosObj *)dict_obj);
    }

    return NULL;
}

CosClass *
cos_dict_obj_class(void)
{
    if (!cos_dict_obj_class_initializer_.initialized) {
        cos_dict_obj_class_initializer_.initialized = true;

        cos_dict_obj_class_init_(cos_dict_obj_class_initializer_.class);
    }

    return (CosClass *)&cos_dict_obj_class_initializer_.class;
}

bool
cos_dict_obj_set(CosDictObj *dict_obj,
                 CosBaseObj *key,
                 CosBaseObj *value,
                 CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(dict_obj != NULL);
    COS_PARAMETER_ASSERT(key != NULL);
    COS_PARAMETER_ASSERT(value != NULL);

    return false;
}

COS_ASSUME_NONNULL_END
