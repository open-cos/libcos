/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/objects/CosStreamObj.h"

#include "common/Assert.h"

#include "libcos/common/CosError.h"

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

    cos_dict_obj_destroy(stream_obj->dict_obj);
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

CosArrayObj *
cos_stream_obj_get_filter_names(const CosStreamObj *stream_obj,
                                CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(stream_obj != NULL);
    if (!stream_obj) {
        return NULL;
    }

    (void)out_error;

    return NULL;
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

    CosObj *length_obj = NULL;
    if (!cos_dict_obj_get_value_with_string(stream_obj->dict_obj,
                                            "DL",
                                            &length_obj,
                                            out_error)) {
        return false;
    }
    else if (!cos_obj_is_integer(length_obj)) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_PARSE,
                                           "The /DL entry in the stream dictionary is not an integer"));
        return false;
    }

    *out_length_hint = 0;

    return true;
}

COS_ASSUME_NONNULL_END
