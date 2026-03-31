/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/objects/CosObjNode.h"

#include "common/Assert.h"

#include "libcos/objects/CosArrayObjNode.h"
#include "libcos/objects/CosBoolObjNode.h"
#include "libcos/objects/CosDictObjNode.h"
#include "libcos/objects/CosIndirectObjNode.h"
#include "libcos/objects/CosIntObjNode.h"
#include "libcos/objects/CosNameObjNode.h"
#include "libcos/objects/CosNullObjNode.h"
#include "libcos/objects/CosRealObjNode.h"
#include "libcos/objects/CosReferenceObjNode.h"
#include "libcos/objects/CosStreamObjNode.h"
#include "libcos/objects/CosStringObjNode.h"

#include <stdio.h>

COS_ASSUME_NONNULL_BEGIN

struct CosObjNode {
    CosObjNodeType type;

    unsigned int ref_count;
};

CosObjNode * COS_Nullable
cos_obj_node_retain(CosObjNode * COS_Nullable obj)
{
    if (COS_UNLIKELY(!obj)) {
        return NULL;
    }

    // The null singleton has no ref_count and must not be retained.
    if (obj->type == CosObjNodeType_Null) {
        return obj;
    }

    obj->ref_count++;
    return obj;
}

static void
cos_obj_dealloc_(CosObjNode *obj)
{
    COS_IMPL_PARAM_CHECK(obj != NULL);

    switch (obj->type) {
        case CosObjNodeType_Unknown:
            break;

        case CosObjNodeType_Boolean: {
            cos_bool_obj_node_free((CosBoolObjNode *)obj);
        } break;

        case CosObjNodeType_Integer: {
            cos_int_obj_node_free((CosIntObjNode *)obj);
        } break;
        case CosObjNodeType_Real: {
            cos_real_obj_node_free((CosRealObjNode *)obj);
        } break;

        case CosObjNodeType_String: {
            cos_string_obj_node_free((CosStringObjNode *)obj);
        } break;

        case CosObjNodeType_Name: {
            cos_name_obj_node_free((CosNameObjNode *)obj);
        } break;

        case CosObjNodeType_Array: {
            cos_array_obj_node_free((CosArrayObjNode *)obj);
        } break;

        case CosObjNodeType_Dict: {
            cos_dict_obj_node_destroy((CosDictObjNode *)obj);
        } break;

        case CosObjNodeType_Stream: {
            cos_stream_obj_node_destroy((CosStreamObjNode *)obj);
        } break;

        case CosObjNodeType_Null: {
            // Nothing to do - the null object cannot be freed.
        } break;

        case CosObjNodeType_Indirect: {
            cos_indirect_obj_node_free((CosIndirectObjNode *)obj);
        } break;

        case CosObjNodeType_Reference: {
            cos_reference_obj_node_free((CosReferenceObjNode *)obj);
        } break;
    }
}

void
cos_obj_node_release(CosObjNode * COS_Nullable obj)
{
    if (!obj) {
        return;
    }

    // The null singleton has no ref_count and must not be freed.
    if (obj->type == CosObjNodeType_Null) {
        return;
    }

    if (obj->ref_count > 1) {
        obj->ref_count--;
        return;
    }

    cos_obj_dealloc_(COS_nonnull_cast(obj));
}

CosObjNodeType
cos_obj_node_get_type(const CosObjNode *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (!obj) {
        return CosObjNodeType_Unknown;
    }

    return obj->type;
}

bool
cos_obj_node_is_direct(const CosObjNode *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (!obj) {
        return false;
    }

    switch (obj->type) {
        case CosObjNodeType_Unknown:
            return false;

        case CosObjNodeType_Boolean:
        case CosObjNodeType_Integer:
        case CosObjNodeType_Real:
        case CosObjNodeType_String:
        case CosObjNodeType_Name:
        case CosObjNodeType_Array:
        case CosObjNodeType_Dict:
        case CosObjNodeType_Stream:
        case CosObjNodeType_Null:
            return true;

        case CosObjNodeType_Indirect:
        case CosObjNodeType_Reference:
            return false;
    }
    COS_ASSERT(false, "Invalid obj type: %d", (int)obj->type);

    return false;
}

bool
cos_obj_node_is_indirect(const CosObjNode *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (!obj) {
        return false;
    }

    switch (obj->type) {
        case CosObjNodeType_Unknown:
        case CosObjNodeType_Boolean:
        case CosObjNodeType_Integer:
        case CosObjNodeType_Real:
        case CosObjNodeType_String:
        case CosObjNodeType_Name:
        case CosObjNodeType_Array:
        case CosObjNodeType_Dict:
        case CosObjNodeType_Stream:
        case CosObjNodeType_Null:
            return false;

        case CosObjNodeType_Indirect:
        case CosObjNodeType_Reference:
            return true;
    }
    COS_ASSERT(false, "Invalid obj type: %d", (int)obj->type);

    return false;
}

bool
cos_obj_node_is_type(const CosObjNode *obj,
                CosObjNodeType type)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (!obj) {
        return false;
    }

    return cos_obj_node_get_type(obj) == type;
}

CosObjNodeValueType
cos_obj_node_get_value_type(CosObjNode *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (!obj) {
        return CosObjNodeValueType_Unknown;
    }

    switch (obj->type) {
        case CosObjNodeType_Unknown:
            return CosObjNodeValueType_Unknown;

        case CosObjNodeType_Boolean:
            return CosObjNodeValueType_Boolean;

        case CosObjNodeType_Integer:
            return CosObjNodeValueType_Integer;

        case CosObjNodeType_Real:
            return CosObjNodeValueType_Real;

        case CosObjNodeType_String:
            return CosObjNodeValueType_String;

        case CosObjNodeType_Name:
            return CosObjNodeValueType_Name;

        case CosObjNodeType_Array:
            return CosObjNodeValueType_Array;

        case CosObjNodeType_Dict:
            return CosObjNodeValueType_Dict;

        case CosObjNodeType_Stream:
            return CosObjNodeValueType_Stream;

        case CosObjNodeType_Null:
            return CosObjNodeValueType_Null;

        case CosObjNodeType_Indirect: {
            return cos_indirect_obj_node_get_type((CosIndirectObjNode *)obj);
        }
        case CosObjNodeType_Reference: {
            return cos_reference_obj_node_get_type((CosReferenceObjNode *)obj);
        }
    }
    COS_ASSERT(false, "Invalid obj type: %d", (int)obj->type);

    return CosObjNodeValueType_Unknown;
}

bool
cos_obj_node_is_boolean(CosObjNode *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (!obj) {
        return false;
    }

    return cos_obj_node_get_value_type(obj) == CosObjNodeValueType_Boolean;
}

bool
cos_obj_node_is_integer(CosObjNode *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (!obj) {
        return false;
    }

    return cos_obj_node_get_value_type(obj) == CosObjNodeValueType_Integer;
}

bool
cos_obj_node_is_real(CosObjNode *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (!obj) {
        return false;
    }

    return cos_obj_node_get_value_type(obj) == CosObjNodeValueType_Real;
}

bool
cos_obj_node_is_string(CosObjNode *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (!obj) {
        return false;
    }

    return cos_obj_node_get_value_type(obj) == CosObjNodeValueType_String;
}

bool
cos_obj_node_is_name(CosObjNode *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (!obj) {
        return false;
    }

    return cos_obj_node_get_value_type(obj) == CosObjNodeValueType_Name;
}

bool
cos_obj_node_is_array(CosObjNode *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (!obj) {
        return false;
    }

    return cos_obj_node_get_value_type(obj) == CosObjNodeValueType_Array;
}

bool
cos_obj_node_is_dict(CosObjNode *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (!obj) {
        return false;
    }

    return cos_obj_node_get_value_type(obj) == CosObjNodeValueType_Dict;
}

bool
cos_obj_node_is_stream(CosObjNode *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (!obj) {
        return false;
    }

    return cos_obj_node_get_value_type(obj) == CosObjNodeValueType_Stream;
}

static void
COS_LOG(const char *message)
{
    printf("%s\n", message);
}

void
cos_obj_node_print_desc(const CosObjNode *obj)
{
    COS_API_PARAM_CHECK(obj != NULL);
    if (!obj) {
        return;
    }

    switch (obj->type) {
        case CosObjNodeType_Unknown:
            COS_LOG("Unknown object");
            break;

        case CosObjNodeType_Boolean: {
            cos_bool_obj_node_print_desc((const CosBoolObjNode *)obj);
        } break;

        case CosObjNodeType_Integer: {
            cos_int_obj_node_print_desc((const CosIntObjNode *)obj);
        } break;

        case CosObjNodeType_Real: {
            cos_real_obj_node_print_desc((const CosRealObjNode *)obj);
        } break;

        case CosObjNodeType_String: {
            cos_string_obj_node_print_desc((const CosStringObjNode *)obj);
        } break;

        case CosObjNodeType_Name: {
            cos_name_obj_node_print_desc((const CosNameObjNode *)obj);
        } break;

        case CosObjNodeType_Array:
            COS_LOG("Array object");
            break;

        case CosObjNodeType_Dict:
            COS_LOG("Dictionary object");
            break;

        case CosObjNodeType_Stream:
            COS_LOG("Stream object");
            break;

        case CosObjNodeType_Null: {
            cos_null_obj_node_print_desc((const CosNullObjNode *)obj);
        } break;

        case CosObjNodeType_Indirect: {
            cos_indirect_obj_node_print_desc((const CosIndirectObjNode *)obj);
        } break;

        case CosObjNodeType_Reference: {
            cos_reference_obj_node_print_desc((const CosReferenceObjNode *)obj);
        } break;
    }
}

COS_ASSUME_NONNULL_END
