//
// Created by david on 18/05/23.
//

#include "libcos/CosBaseObj.h"

#include "common/Assert.h"
#include "libcos/common/CosClass.h"
#include "libcos/private/common/CosClass-Impl.h"
#include "libcos/private/objects/CosBaseObj-Impl.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

static CosClass cos_base_obj_class_;

struct CosClassInitializer {
    CosClass *class;
    bool initialized;
};

static struct CosClassInitializer cos_obj_class_initializer_ = {
    .class = &cos_base_obj_class_,
    .initialized = false,
};

CosClass *
cos_base_obj_class(void)
{
    if (!cos_obj_class_initializer_.initialized) {
        cos_obj_class_initializer_.initialized = true;

        cos_class_init(cos_obj_class_initializer_.class,
                       cos_obj_root_class(),
                       NULL,
                       NULL);
    }

    return cos_obj_class_initializer_.class;
}

void *
cos_obj_alloc(size_t size,
              CosObjectType type,
              CosDoc * COS_Nullable document)
{
    CosBaseObj * const obj = malloc(size);
    if (!obj) {
        return NULL;
    }

    return obj;
}

COS_ASSUME_NONNULL_END
