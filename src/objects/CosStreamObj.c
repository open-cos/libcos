/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/objects/CosStreamObj.h"

#include "common/Assert.h"

#include <libcos/common/CosData.h>
#include <libcos/objects/CosDictObj.h>
#include <libcos/objects/CosObj.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosStreamObj {
    CosObjType type;

    CosDictObj *dict_obj;
    CosData * COS_Nullable data;
};

CosStreamObj *
cos_stream_obj_create(CosDictObj *dict,
                      CosData * COS_Nullable data)
{
    CosStreamObj * const stream_obj = malloc(sizeof(CosStreamObj));
    if (!stream_obj) {
        return NULL;
    }

    stream_obj->type = CosObjType_Stream;
    stream_obj->dict_obj = dict;
    stream_obj->data = data;

    return stream_obj;
}

void
cos_stream_obj_destroy(CosStreamObj *stream_obj)
{
    if (!stream_obj) {
        return;
    }

    cos_dict_obj_free(stream_obj->dict_obj);
    if (stream_obj->data) {
        cos_data_free(COS_nonnull_cast(stream_obj->data));
    }
    free(stream_obj);
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

size_t
cos_stream_obj_get_length(const CosStreamObj *stream_obj)
{
    COS_PARAMETER_ASSERT(stream_obj != NULL);
    if (!stream_obj) {
        return 0;
    }

    return stream_obj->data->size;
}

bool
cos_stream_obj_get_decoded_length_hint(const CosStreamObj *stream_obj,
                                       size_t *out_length_hint,
                                       CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(stream_obj != NULL);
    COS_PARAMETER_ASSERT(out_length_hint != NULL);
    if (!stream_obj || !out_length_hint) {
        return false;
    }

    *out_length_hint = 0;

    return true;
}

COS_ASSUME_NONNULL_END
