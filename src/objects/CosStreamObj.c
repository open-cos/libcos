/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/objects/CosStreamObj.h"

#include "common/Assert.h"
#include "libcos/private/objects/CosStreamObj-Impl.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

CosStreamObj *
cos_stream_obj_alloc(CosDictObj *dict_obj,
                     CosData * COS_Nullable data)
{
    CosStreamObj * const stream_obj = malloc(sizeof(CosStreamObj));
    if (!stream_obj) {
        return NULL;
    }

    stream_obj->dict_obj = dict_obj;
    stream_obj->data = data;

    return stream_obj;
}

CosDictObj *
cos_stream_obj_get_dict(const CosStreamObj *stream_obj)
{
    COS_PARAMETER_ASSERT(stream_obj != NULL);
    if (!stream_obj) {
        return NULL;
    }

    return stream_obj->dict_obj;
}

CosData *
cos_stream_obj_get_data(const CosStreamObj *stream_obj)
{
    COS_PARAMETER_ASSERT(stream_obj != NULL);
    if (!stream_obj) {
        return NULL;
    }

    return stream_obj->data;
}

COS_ASSUME_NONNULL_END
