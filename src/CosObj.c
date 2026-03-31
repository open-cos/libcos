/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/CosObj.h"

#include "common/Assert.h"

#include "libcos/common/CosData.h"
#include "libcos/common/CosError.h"
#include "libcos/common/CosString.h"
#include "libcos/objects/CosArrayObjNode.h"
#include "libcos/objects/CosBoolObjNode.h"
#include "libcos/objects/CosDictObjNode.h"
#include "libcos/objects/CosIndirectObjNode.h"
#include "libcos/objects/CosIntObjNode.h"
#include "libcos/objects/CosNameObjNode.h"
#include "libcos/objects/CosNullObjNode.h"
#include "libcos/objects/CosObjNode.h"
#include "libcos/objects/CosRealObjNode.h"
#include "libcos/objects/CosReferenceObjNode.h"
#include "libcos/objects/CosStreamObjNode.h"
#include "libcos/objects/CosStringObjNode.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosObj {
    CosObjNode *node;
};

// MARK: - Private helpers

/**
 * Resolves through Indirect and Reference wrappers to the direct node.
 *
 * The returned pointer is borrowed from the wrapper -- it remains valid
 * as long as the wrapper (and thus the CosObj) is alive.
 */
static CosObjNode * COS_Nullable
cos_obj_get_direct_node_(const CosObj *obj)
{
    COS_IMPL_PARAM_CHECK(obj != NULL);

    CosObjNode * const node = obj->node;
    const CosObjNodeType type = cos_obj_node_get_type(node);

    if (type == CosObjNodeType_Indirect) {
        return cos_indirect_obj_node_get_value((CosIndirectObjNode *)node);
    }
    if (type == CosObjNodeType_Reference) {
        return cos_reference_obj_node_get_value((CosReferenceObjNode *)node);
    }
    return node;
}

static CosObjType
cos_obj_type_from_node_type_(CosObjNodeType node_type)
{
    switch (node_type) {
        case CosObjNodeType_Boolean:
            return CosObjType_Boolean;
        case CosObjNodeType_Integer:
            return CosObjType_Integer;
        case CosObjNodeType_Real:
            return CosObjType_Real;
        case CosObjNodeType_String:
            return CosObjType_String;
        case CosObjNodeType_Name:
            return CosObjType_Name;
        case CosObjNodeType_Array:
            return CosObjType_Array;
        case CosObjNodeType_Dict:
            return CosObjType_Dict;
        case CosObjNodeType_Stream:
            return CosObjType_Stream;
        case CosObjNodeType_Null:
            return CosObjType_Null;
        case CosObjNodeType_Unknown:
        case CosObjNodeType_Indirect:
        case CosObjNodeType_Reference:
            return CosObjType_Unknown;
    }
    return CosObjType_Unknown;
}

// MARK: - Lifecycle

CosObj * COS_Nullable
cos_obj_create(CosObjNode *node)
{
    COS_API_PARAM_CHECK(node != NULL);
    if (COS_UNLIKELY(!node)) {
        return NULL;
    }

    CosObj * const obj = malloc(sizeof(CosObj));
    if (COS_UNLIKELY(!obj)) {
        return NULL;
    }

    cos_obj_node_retain(node);
    obj->node = node;

    return obj;
}

void
cos_obj_destroy(CosObj *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (COS_UNLIKELY(!obj)) {
        return;
    }

    cos_obj_node_release(obj->node);
    free(obj);
}

// MARK: - Type query

CosObjType
cos_obj_get_type(const CosObj *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (COS_UNLIKELY(!obj)) {
        return CosObjType_Unknown;
    }

    CosObjNode * const direct = cos_obj_get_direct_node_(obj);
    if (!direct) {
        return CosObjType_Unknown;
    }
    return cos_obj_type_from_node_type_(cos_obj_node_get_type(direct));
}

bool
cos_obj_is_direct(const CosObj *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (COS_UNLIKELY(!obj)) {
        return false;
    }

    return cos_obj_node_is_direct(obj->node);
}

bool
cos_obj_is_indirect(const CosObj *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (COS_UNLIKELY(!obj)) {
        return false;
    }

    return cos_obj_node_is_indirect(obj->node);
}

bool
cos_obj_is_null(const CosObj *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (COS_UNLIKELY(!obj)) {
        return false;
    }

    return cos_obj_get_type(obj) == CosObjType_Null;
}

// MARK: - Scalar accessors

bool
cos_obj_get_bool_value(const CosObj *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (COS_UNLIKELY(!obj)) {
        return false;
    }

    CosObjNode * const direct = cos_obj_get_direct_node_(obj);
    if (!direct || cos_obj_node_get_type(direct) != CosObjNodeType_Boolean) {
        return false;
    }
    return cos_bool_obj_node_get_value((CosBoolObjNode *)direct);
}

int
cos_obj_get_int_value(const CosObj *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (COS_UNLIKELY(!obj)) {
        return 0;
    }

    CosObjNode * const direct = cos_obj_get_direct_node_(obj);
    if (!direct || cos_obj_node_get_type(direct) != CosObjNodeType_Integer) {
        return 0;
    }
    return cos_int_obj_node_get_value((CosIntObjNode *)direct);
}

double
cos_obj_get_real_value(const CosObj *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (COS_UNLIKELY(!obj)) {
        return 0.0;
    }

    CosObjNode * const direct = cos_obj_get_direct_node_(obj);
    if (!direct || cos_obj_node_get_type(direct) != CosObjNodeType_Real) {
        return 0.0;
    }
    return cos_real_obj_node_get_value((CosRealObjNode *)direct);
}

// MARK: - String / Name accessors

const CosData * COS_Nullable
cos_obj_get_string_value(const CosObj *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (COS_UNLIKELY(!obj)) {
        return NULL;
    }

    CosObjNode * const direct = cos_obj_get_direct_node_(obj);
    if (!direct || cos_obj_node_get_type(direct) != CosObjNodeType_String) {
        return NULL;
    }
    return cos_string_obj_node_get_value((CosStringObjNode *)direct);
}

const CosString * COS_Nullable
cos_obj_get_name_value(const CosObj *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (COS_UNLIKELY(!obj)) {
        return NULL;
    }

    CosObjNode * const direct = cos_obj_get_direct_node_(obj);
    if (!direct || cos_obj_node_get_type(direct) != CosObjNodeType_Name) {
        return NULL;
    }
    return cos_name_obj_node_get_value((CosNameObjNode *)direct);
}

// MARK: - Array accessors

size_t
cos_obj_get_array_count(const CosObj *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (COS_UNLIKELY(!obj)) {
        return 0;
    }

    CosObjNode * const direct = cos_obj_get_direct_node_(obj);
    if (!direct || cos_obj_node_get_type(direct) != CosObjNodeType_Array) {
        return 0;
    }
    return cos_array_obj_node_get_count((CosArrayObjNode *)direct);
}

CosObj * COS_Nullable
cos_obj_get_object_at_index(const CosObj *obj,
                            size_t index,
                            CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (COS_UNLIKELY(!obj)) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Object is NULL"));
        return NULL;
    }

    CosObjNode * const direct = cos_obj_get_direct_node_(obj);
    if (!direct || cos_obj_node_get_type(direct) != CosObjNodeType_Array) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Object is not an array"));
        return NULL;
    }

    CosObjNode * const element =
        cos_array_obj_node_get_at((CosArrayObjNode *)direct,
                                  index,
                                  out_error);
    if (!element) {
        return NULL;
    }

    return cos_obj_create(element);
}

// MARK: - Dict accessors

size_t
cos_obj_get_dict_count(const CosObj *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (COS_UNLIKELY(!obj)) {
        return 0;
    }

    CosObjNode * const direct = cos_obj_get_direct_node_(obj);
    if (!direct || cos_obj_node_get_type(direct) != CosObjNodeType_Dict) {
        return 0;
    }
    return cos_dict_obj_node_get_count((CosDictObjNode *)direct);
}

CosObj * COS_Nullable
cos_obj_get_value_for_key(const CosObj *obj,
                          const char *key,
                          CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(obj != NULL);
    COS_API_PARAM_CHECK(key != NULL);
    if (COS_UNLIKELY(!obj || !key)) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Object or key is NULL"));
        return NULL;
    }

    CosObjNode * const direct = cos_obj_get_direct_node_(obj);
    if (!direct || cos_obj_node_get_type(direct) != CosObjNodeType_Dict) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Object is not a dictionary"));
        return NULL;
    }

    CosObjNode *value = NULL;
    if (!cos_dict_obj_node_get_value_with_string((CosDictObjNode *)direct,
                                                 key,
                                                 &value,
                                                 out_error)) {
        return NULL;
    }

    return cos_obj_create(value);
}

// MARK: - Dict iterator

CosObjDictIterator
cos_obj_dict_iterator_init(const CosObj *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);

    CosObjNode * const direct = cos_obj_get_direct_node_(obj);
    if (!direct || cos_obj_node_get_type(direct) != CosObjNodeType_Dict) {
        CosObjDictIterator iter = {0};
        return iter;
    }

    return (CosObjDictIterator){
        .base = cos_dict_obj_node_iterator_init((CosDictObjNode *)direct),
    };
}

bool
cos_obj_dict_iterator_next(CosObjDictIterator *iterator,
                           const char * COS_Nullable * COS_Nonnull out_key,
                           CosObj * COS_Nullable * COS_Nonnull out_value)
{
    COS_API_PARAM_CHECK(iterator != NULL);
    COS_API_PARAM_CHECK(out_key != NULL);
    COS_API_PARAM_CHECK(out_value != NULL);
    if (COS_UNLIKELY(!iterator || !out_key || !out_value)) {
        return false;
    }

    CosNameObjNode *name_node = NULL;
    CosObjNode *value_node = NULL;

    if (!cos_dict_obj_node_iterator_next(&iterator->base,
                                         &name_node,
                                         &value_node)) {
        *out_key = NULL;
        *out_value = NULL;
        return false;
    }

    const CosString * const name_str = cos_name_obj_node_get_value(name_node);
    *out_key = name_str ? cos_string_get_data(name_str) : NULL;
    *out_value = cos_obj_create(value_node);

    return true;
}

// MARK: - Stream accessors

CosObj * COS_Nullable
cos_obj_get_stream_dict(const CosObj *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (COS_UNLIKELY(!obj)) {
        return NULL;
    }

    CosObjNode * const direct = cos_obj_get_direct_node_(obj);
    if (!direct || cos_obj_node_get_type(direct) != CosObjNodeType_Stream) {
        return NULL;
    }

    CosDictObjNode * const dict =
        cos_stream_obj_node_get_dict((CosStreamObjNode *)direct);
    if (!dict) {
        return NULL;
    }

    return cos_obj_create((CosObjNode *)dict);
}

const CosData * COS_Nullable
cos_obj_get_stream_data(const CosObj *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (COS_UNLIKELY(!obj)) {
        return NULL;
    }

    CosObjNode * const direct = cos_obj_get_direct_node_(obj);
    if (!direct || cos_obj_node_get_type(direct) != CosObjNodeType_Stream) {
        return NULL;
    }

    return cos_stream_obj_node_get_data((CosStreamObjNode *)direct);
}

COS_ASSUME_NONNULL_END
