/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/objects/CosObj.h"

#include "common/Assert.h"
#include "libcos/objects/CosArrayObj.h"
#include "libcos/objects/CosBoolObj.h"
#include "libcos/objects/CosDictObj.h"
#include "libcos/objects/CosIndirectObj.h"
#include "libcos/objects/CosIntObj.h"
#include "libcos/objects/CosNameObj.h"
#include "libcos/objects/CosRealObj.h"
#include "libcos/objects/CosReferenceObj.h"
#include "libcos/objects/CosStreamObj.h"
#include "libcos/objects/CosStringObj.h"

#include <stdio.h>

COS_ASSUME_NONNULL_BEGIN

struct CosObj {
    CosObjType type;

    COS_ATTR_UNUSED unsigned char padding_[4];
};

void
cos_obj_free(CosObj *obj)
{
    if (!obj) {
        return;
    }

    switch (obj->type) {
        case CosObjType_Unknown:
            break;

        case CosObjType_Boolean: {
            cos_bool_obj_free((CosBoolObj *)obj);
        } break;

        case CosObjType_Integer: {
            cos_int_obj_free((CosIntObj *)obj);
        } break;
        case CosObjType_Real: {
            cos_real_obj_free((CosRealObj *)obj);
        } break;

        case CosObjType_String: {
            cos_string_obj_free((CosStringObj *)obj);
        } break;

        case CosObjType_Name: {
            cos_name_obj_free((CosNameObj *)obj);
        } break;

        case CosObjType_Array: {
            cos_array_obj_free((CosArrayObj *)obj);
        } break;

        case CosObjType_Dict: {
            cos_dict_obj_free((CosDictObj *)obj);
        } break;

        case CosObjType_Stream: {
            cos_stream_obj_free((CosStreamObj *)obj);
        } break;

        case CosObjType_Null: {
            // Nothing to do - the null object cannot be freed.
        } break;

        case CosObjType_Indirect: {
            cos_indirect_obj_free((CosIndirectObj *)obj);
        } break;

        case CosObjType_Reference: {
            cos_reference_obj_free((CosReferenceObj *)obj);
        } break;
    }
}

CosObjType
cos_obj_get_type(const CosObj *obj)
{
    COS_PARAMETER_ASSERT(obj != NULL);
    if (!obj) {
        return CosObjType_Unknown;
    }

    return obj->type;
}

bool
cos_obj_is_direct(const CosObj *obj)
{
    COS_PARAMETER_ASSERT(obj != NULL);
    if (!obj) {
        return false;
    }

    switch (obj->type) {
        case CosObjType_Unknown:
            return false;

        case CosObjType_Boolean:
        case CosObjType_Integer:
        case CosObjType_Real:
        case CosObjType_String:
        case CosObjType_Name:
        case CosObjType_Array:
        case CosObjType_Dict:
        case CosObjType_Stream:
        case CosObjType_Null:
            return true;

        case CosObjType_Indirect:
        case CosObjType_Reference:
            return false;
    }
    COS_ASSERT(false, "Invalid obj type: %d", obj->type);

    return false;
}

bool
cos_obj_is_indirect(const CosObj *obj)
{
    COS_PARAMETER_ASSERT(obj != NULL);
    if (!obj) {
        return false;
    }

    switch (obj->type) {
        case CosObjType_Unknown:
        case CosObjType_Boolean:
        case CosObjType_Integer:
        case CosObjType_Real:
        case CosObjType_String:
        case CosObjType_Name:
        case CosObjType_Array:
        case CosObjType_Dict:
        case CosObjType_Stream:
        case CosObjType_Null:
            return false;

        case CosObjType_Indirect:
        case CosObjType_Reference:
            return true;
    }
    COS_ASSERT(false, "Invalid obj type: %d", obj->type);

    return false;
}

CosObjValueType
cos_obj_get_value_type(CosObj *obj)
{
    COS_PARAMETER_ASSERT(obj != NULL);
    if (!obj) {
        return CosObjValueType_Unknown;
    }

    switch (obj->type) {
        case CosObjType_Unknown:
            return CosObjValueType_Unknown;

        case CosObjType_Boolean:
            return CosObjValueType_Boolean;

        case CosObjType_Integer:
            return CosObjValueType_Integer;

        case CosObjType_Real:
            return CosObjValueType_Real;

        case CosObjType_String:
            return CosObjValueType_String;

        case CosObjType_Name:
            return CosObjValueType_Name;

        case CosObjType_Array:
            return CosObjValueType_Array;

        case CosObjType_Dict:
            return CosObjValueType_Dictionary;

        case CosObjType_Stream:
            return CosObjValueType_Stream;

        case CosObjType_Null:
            return CosObjValueType_Null;

        case CosObjType_Indirect: {
            return cos_indirect_obj_get_type((CosIndirectObj *)obj);
        }
        case CosObjType_Reference: {
            return cos_reference_obj_get_type((CosReferenceObj *)obj);
        }
    }
    COS_ASSERT(false, "Invalid obj type: %d", obj->type);

    return CosObjValueType_Unknown;
}

static void
COS_LOG(const char *message)
{
    printf("%s\n", message);
}

void
cos_obj_print_description(const CosObj *obj)
{
    COS_PARAMETER_ASSERT(obj != NULL);
    if (!obj) {
        return;
    }

    switch (obj->type) {
        case CosObjType_Unknown:
            COS_LOG("Unknown object");
            break;

        case CosObjType_Boolean:
            COS_LOG("Boolean object");
            break;

        case CosObjType_Integer:
            COS_LOG("Integer object");
            break;

        case CosObjType_Real:
            COS_LOG("Real object");
            break;

        case CosObjType_String:
            COS_LOG("String object");
            break;

        case CosObjType_Name:
            COS_LOG("Name object");
            break;

        case CosObjType_Array:
            COS_LOG("Array object");
            break;

        case CosObjType_Dict:
            COS_LOG("Dictionary object");
            break;

        case CosObjType_Stream:
            COS_LOG("Stream object");
            break;

        case CosObjType_Null:
            COS_LOG("Null object");
            break;

        case CosObjType_Indirect:
            COS_LOG("Indirect object");
            break;

        case CosObjType_Reference:
            COS_LOG("Reference object");
            break;
    }
}

COS_ASSUME_NONNULL_END
