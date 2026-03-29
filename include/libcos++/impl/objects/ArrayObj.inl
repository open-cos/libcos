/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_IMPL_COMMON_ARRAY_OBJ_INL
#define LIBCOS_IMPL_COMMON_ARRAY_OBJ_INL

#include "common/CosError.h"

#include <libcos++/objects/ArrayObj.hpp>
#include <libcos/objects/CosArrayObjNode.h>

namespace opencos {

ArrayObj::~ArrayObj() = default;

std::size_t
ArrayObj::getCount() const
{
    return cos_array_obj_node_get_count(getArrayImpl());
}

Obj
ArrayObj::getAt(std::size_t index) const
{
    CosError error;
    CosObjNode * const obj = cos_array_obj_node_get_at(getArrayImpl(),
                                              index,
                                              &error);
    if (obj == nullptr) {
        return Obj(nullptr);
    }

    return Obj(obj);
}

ArrayObj::ArrayObj(CosArrayObjNode *impl)
    : Obj(reinterpret_cast<CosObjNode *>(impl))
{}

CosArrayObjNode *
ArrayObj::getArrayImpl() const
{
    return reinterpret_cast<CosArrayObjNode *>(Obj::getImpl());
}

} // namespace opencos

#endif /* LIBCOS_IMPL_COMMON_ARRAY_OBJ_INL */
