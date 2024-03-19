/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_IMPL_COMMON_ARRAY_OBJ_INL
#define LIBCOS_IMPL_COMMON_ARRAY_OBJ_INL

#include "common/CosError.h"

#include <libcos++/objects/ArrayObj.hpp>
#include <libcos/objects/CosArrayObj.h>

namespace opencos {

ArrayObj::~ArrayObj() = default;

std::size_t
ArrayObj::getCount() const
{
    return cos_array_obj_get_count(getArrayImpl());
}

Obj
ArrayObj::getAt(std::size_t index) const
{
    CosError error;
    CosObj * const obj = cos_array_obj_get_at(getArrayImpl(),
                                              index,
                                              &error);
    if (obj == nullptr) {
        return Obj(nullptr);
    }

    return Obj(obj);
}

ArrayObj::ArrayObj(CosArrayObj *impl)
    : Obj(reinterpret_cast<CosObj *>(impl))
{}

CosArrayObj *
ArrayObj::getArrayImpl() const
{
    return reinterpret_cast<CosArrayObj *>(Obj::getImpl());
}

} // namespace opencos

#endif /* LIBCOS_IMPL_COMMON_ARRAY_OBJ_INL */
