/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/objects/CosStreamObjNode.h"

#include "common/Assert.h"
#include "filters/CosFilterFactory.h"
#include "filters/CosPredictorFilter.h"

#include "libcos/common/CosError.h"

#include <libcos/common/CosData.h>
#include <libcos/common/CosString.h>
#include <libcos/filters/CosFilter.h>
#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>
#include <libcos/objects/CosArrayObjNode.h>
#include <libcos/objects/CosDictObjNode.h>
#include <libcos/objects/CosIntObjNode.h>
#include <libcos/objects/CosNameObjNode.h>
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

// Reads an integer entry from a /DecodeParms dictionary, or returns the default
// when the params are absent, not a dictionary, or the entry is not an integer.
static int
cos_stream_parms_int_(CosObjNode * COS_Nullable parms,
                      const char *key,
                      int default_value)
{
    if (!parms || cos_obj_node_get_type(parms) != CosObjNodeType_Dict) {
        return default_value;
    }

    CosObjNode *value = NULL;
    if (!cos_dict_obj_node_get_value_with_string((CosDictObjNode *)parms,
                                            key,
                                            &value,
                                            NULL)) {
        return default_value;
    }
    if (!value || !cos_obj_node_is_integer(value)) {
        return default_value;
    }

    return cos_int_obj_node_get_value((CosIntObjNode *)value);
}

// Returns the /DecodeParms entry that applies to filter @p index: the i-th array
// element when the params are an array, the single dictionary for the first
// filter, or NULL.
static CosObjNode * COS_Nullable
cos_stream_parms_for_index_(CosObjNode * COS_Nullable parms_node,
                            size_t index)
{
    if (!parms_node) {
        return NULL;
    }
    if (cos_obj_node_get_type(parms_node) == CosObjNodeType_Array) {
        return cos_array_obj_node_get_at((CosArrayObjNode *)parms_node, index, NULL);
    }
    return (index == 0) ? parms_node : NULL;
}

// Wraps a single filter (and, if its /DecodeParms requests one, a predictor)
// around the source stream, closing the source on failure. Returns the new top
// of the chain, or NULL on error.
static CosStream * COS_Nullable
cos_stream_obj_node_apply_filter_(CosObjNode *name_node,
                                  CosObjNode * COS_Nullable parms_node,
                                  CosStream *source,
                                  CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(name_node != NULL);
    COS_IMPL_PARAM_CHECK(source != NULL);

    if (cos_obj_node_get_type(name_node) != CosObjNodeType_Name) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_NOT_IMPLEMENTED,
                                           "Indirect or non-name /Filter entry is not supported"));
        goto failure;
    }

    const CosString * const name = cos_name_obj_node_get_value((CosNameObjNode *)name_node);
    if (!name) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_SYNTAX,
                                           "Empty /Filter name"));
        goto failure;
    }

    CosStream * const filter = cos_filter_create_for_name_(name, out_error);
    if (!filter) {
        goto failure;
    }

    // From here the filter owns the source; failures must close the filter,
    // which cascades to close its source chain.
    cos_filter_attach_source((CosFilter *)filter, source);

    // Apply a predictor post-process if /DecodeParms requests one.
    const int predictor = cos_stream_parms_int_(parms_node, "Predictor", 1);
    if (predictor > 1) {
        const int colors = cos_stream_parms_int_(parms_node, "Colors", 1);
        const int bits_per_component = cos_stream_parms_int_(parms_node, "BitsPerComponent", 8);
        const int columns = cos_stream_parms_int_(parms_node, "Columns", 1);

        CosPredictorFilter * const predictor_filter =
            cos_predictor_filter_create_(predictor,
                                         (colors > 0) ? (size_t)colors : 1,
                                         (bits_per_component > 0) ? (size_t)bits_per_component : 8,
                                         (columns > 0) ? (size_t)columns : 1,
                                         out_error);
        if (!predictor_filter) {
            cos_stream_close(filter);
            return NULL;
        }
        cos_filter_attach_source((CosFilter *)predictor_filter, filter);
        return (CosStream *)predictor_filter;
    }

    return filter;

failure:
    if (source) {
        cos_stream_close(source);
    }
    return NULL;
}

CosStream *
cos_stream_obj_node_create_decode_stream(const CosStreamObjNode *stream_obj,
                                    CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(stream_obj != NULL);
    if (COS_UNLIKELY(!stream_obj)) {
        return NULL;
    }

    // Wrap the encoded bytes in a read-only source stream.
    const unsigned char *bytes = (const unsigned char *)"";
    size_t size = 0;
    if (stream_obj->data) {
        bytes = stream_obj->data->bytes;
        size = stream_obj->data->size;
    }
    CosStream *source = (CosStream *)cos_memory_stream_create_readonly(bytes, size);
    if (COS_UNLIKELY(!source)) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to allocate stream"));
        return NULL;
    }

    // No /Filter: the decoded bytes are the encoded bytes.
    CosObjNode *filter_node = NULL;
    if (!cos_dict_obj_node_get_value_with_string(stream_obj->dict_obj,
                                            "Filter",
                                            &filter_node,
                                            NULL) ||
        !filter_node) {
        return source;
    }

    // The parallel /DecodeParms (or its abbreviation /DP) supplies predictor
    // parameters per filter.
    CosObjNode *parms_node = NULL;
    if (!cos_dict_obj_node_get_value_with_string(stream_obj->dict_obj,
                                            "DecodeParms",
                                            &parms_node,
                                            NULL)) {
        (void)cos_dict_obj_node_get_value_with_string(stream_obj->dict_obj,
                                                 "DP",
                                                 &parms_node,
                                                 NULL);
    }

    const CosObjNodeType filter_type = cos_obj_node_get_type(filter_node);
    if (filter_type == CosObjNodeType_Name) {
        return cos_stream_obj_node_apply_filter_(filter_node, parms_node, source, out_error);
    }
    else if (filter_type == CosObjNodeType_Array) {
        CosArrayObjNode * const array = (CosArrayObjNode *)filter_node;
        const size_t count = cos_array_obj_node_get_count(array);
        for (size_t i = 0; i < count; i++) {
            CosObjNode * const element = cos_array_obj_node_get_at(array, i, out_error);
            if (!element) {
                goto failure;
            }
            CosObjNode * const parms = cos_stream_parms_for_index_(parms_node, i);
            // On failure the filter helper has already closed the source.
            source = cos_stream_obj_node_apply_filter_(element, parms, source, out_error);
            if (!source) {
                return NULL;
            }
        }
        return source;
    }

    cos_error_propagate(out_error,
                        cos_error_make(COS_ERROR_SYNTAX,
                                       "The /Filter entry is not a name or array"));

failure:
    if (source) {
        cos_stream_close(source);
    }
    return NULL;
}

CosData *
cos_stream_obj_node_get_decoded_data(const CosStreamObjNode *stream_obj,
                                CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(stream_obj != NULL);
    if (COS_UNLIKELY(!stream_obj)) {
        return NULL;
    }

    CosStream * const chain = cos_stream_obj_node_create_decode_stream(stream_obj, out_error);
    if (!chain) {
        return NULL;
    }

    size_t hint = 0;
    if (!cos_stream_obj_node_get_decoded_length_hint(stream_obj, &hint, NULL) || hint == 0) {
        hint = stream_obj->data ? stream_obj->data->size : 0;
    }

    CosData *data = cos_data_alloc(hint);
    if (COS_UNLIKELY(!data)) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to allocate decoded data"));
        goto failure;
    }

    unsigned char buffer[4096];
    CosError read_error = {COS_ERROR_NONE, NULL};
    for (;;) {
        const size_t read_count = cos_stream_read(chain, buffer, sizeof(buffer), &read_error);
        if (read_count == 0) {
            break;
        }
        if (!cos_data_append(data, buffer, read_count, out_error)) {
            goto failure;
        }
    }

    if (read_error.code != COS_ERROR_NONE) {
        cos_error_propagate(out_error, read_error);
        goto failure;
    }

    cos_stream_close(chain);
    return data;

failure:
    if (data) {
        cos_data_free(data);
    }
    if (chain) {
        cos_stream_close(chain);
    }
    return NULL;
}

COS_ASSUME_NONNULL_END
