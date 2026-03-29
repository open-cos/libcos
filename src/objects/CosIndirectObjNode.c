/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/objects/CosIndirectObjNode.h"

#include "common/Assert.h"

#include "libcos/objects/CosObjNode.h"

#include <libcos/CosDoc.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosIndirectObjNode {
    /**
     * @c CosObjNodeType_Indirect.
     */
    CosObjNodeType type;
    unsigned int ref_count;

    /**
     * The ID of the indirect object.
     */
    CosObjID id;

    /**
     * The (direct) object value.
     */
    CosObjNode *value;
};

CosIndirectObjNode *
cos_indirect_obj_node_alloc(CosObjID id,
                       CosObjNode *value)
{
    COS_API_PARAM_CHECK(value != NULL);
    if (!value) {
        return NULL;
    }

    CosIndirectObjNode * const indirect_obj = calloc(1, sizeof(CosIndirectObjNode));
    if (!indirect_obj) {
        return NULL;
    }

    indirect_obj->type = CosObjNodeType_Indirect;
    indirect_obj->ref_count = 1;
    indirect_obj->id = id;
    indirect_obj->value = value;

    return indirect_obj;
}

void
cos_indirect_obj_node_free(CosIndirectObjNode *indirect_obj)
{
    if (!indirect_obj) {
        return;
    }

    cos_obj_node_free(indirect_obj->value);

    free(indirect_obj);
}

CosObjID
cos_indirect_obj_node_get_id(const CosIndirectObjNode *indirect_obj)
{
    COS_API_PARAM_CHECK(indirect_obj != NULL);
    if (!indirect_obj) {
        return CosObjID_Invalid;
    }

    return indirect_obj->id;
}

CosObjNode *
cos_indirect_obj_node_get_value(const CosIndirectObjNode *indirect_obj)
{
    COS_API_PARAM_CHECK(indirect_obj != NULL);
    if (!indirect_obj) {
        return NULL;
    }

    return indirect_obj->value;
}

CosObjNodeValueType
cos_indirect_obj_node_get_type(const CosIndirectObjNode *indirect_obj)
{
    COS_API_PARAM_CHECK(indirect_obj != NULL);
    if (!indirect_obj) {
        return CosObjNodeValueType_Unknown;
    }

    const CosObjNode * const direct_obj = indirect_obj->value;
    if (!direct_obj) {
        return CosObjNodeValueType_Unknown;
    }
    return (CosObjNodeValueType)cos_obj_node_get_type(direct_obj);
}

void
cos_indirect_obj_node_print_desc(const CosIndirectObjNode *indirect_obj)
{
    COS_API_PARAM_CHECK(indirect_obj != NULL);
    if (!indirect_obj) {
        return;
    }

    printf("Indirect object: %u %u\n",
           indirect_obj->id.obj_number,
           indirect_obj->id.gen_number);

    const CosObjNode * const direct_obj = indirect_obj->value;
    if (!direct_obj) {
        return;
    }

    cos_obj_node_print_desc(direct_obj);
}

COS_ASSUME_NONNULL_END
