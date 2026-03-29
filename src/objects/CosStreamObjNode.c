/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/objects/CosStreamObjNode.h"

#include "common/Assert.h"

#include "libcos/common/CosError.h"

#include <libcos/common/CosData.h>
#include <libcos/objects/CosDictObjNode.h>
#include <libcos/objects/CosObjNode.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosStreamObjNode {
    CosObjNodeType type;
    unsigned int ref_count;

    CosDictObjNode *dict_obj;
    CosData * COS_Nullable data;
};

CosStreamObjNode *
cos_stream_obj_node_create(CosDictObjNode *dict,
                      CosData * COS_Nullable data)
{
    CosStreamObjNode * const stream_obj = malloc(sizeof(CosStreamObjNode));
    if (!stream_obj) {
        return NULL;
    }

    stream_obj->type = CosObjNodeType_Stream;
    stream_obj->ref_count = 1;
    stream_obj->dict_obj = dict;
    stream_obj->data = data;

    return stream_obj;
}

void
cos_stream_obj_node_destroy(CosStreamObjNode *stream_obj)
{
    if (!stream_obj) {
        return;
    }

    cos_dict_obj_node_destroy(stream_obj->dict_obj);
    if (stream_obj->data) {
        cos_data_free(COS_nonnull_cast(stream_obj->data));
    }
    free(stream_obj);
}

CosDictObjNode *
cos_stream_obj_node_get_dict(const CosStreamObjNode *stream_obj)
{
    COS_API_PARAM_CHECK(stream_obj != NULL);
    if (!stream_obj) {
        return NULL;
    }

    return stream_obj->dict_obj;
}

CosData *
cos_stream_obj_node_get_data(const CosStreamObjNode *stream_obj)
{
    COS_API_PARAM_CHECK(stream_obj != NULL);
    if (!stream_obj) {
        return NULL;
    }

    return stream_obj->data;
}

size_t
cos_stream_obj_node_get_length(const CosStreamObjNode *stream_obj)
{
    COS_API_PARAM_CHECK(stream_obj != NULL);
    if (!stream_obj) {
        return 0;
    }

    return stream_obj->data->size;
}

CosArrayObjNode *
cos_stream_obj_node_get_filter_names(const CosStreamObjNode *stream_obj,
                                CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(stream_obj != NULL);
    if (!stream_obj) {
        return NULL;
    }

    (void)out_error;

    return NULL;
}

bool
cos_stream_obj_node_get_decoded_length_hint(const CosStreamObjNode *stream_obj,
                                       size_t *out_length_hint,
                                       CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(stream_obj != NULL);
    COS_API_PARAM_CHECK(out_length_hint != NULL);
    if (!stream_obj || !out_length_hint) {
        return false;
    }

    CosObjNode *length_obj = NULL;
    if (!cos_dict_obj_node_get_value_with_string(stream_obj->dict_obj,
                                            "DL",
                                            &length_obj,
                                            out_error)) {
        return false;
    }
    else if (!cos_obj_node_is_integer(length_obj)) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_PARSE,
                                           "The /DL entry in the stream dictionary is not an integer"));
        return false;
    }

    *out_length_hint = 0;

    return true;
}

COS_ASSUME_NONNULL_END
