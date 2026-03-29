/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOSXX_OBJECTS_NAME_OBJ_INL
#define LIBCOSXX_OBJECTS_NAME_OBJ_INL

#include "common/CosString.h"

#include <libcos++/objects/NameObj.hpp>
#include <libcos/objects/CosNameObjNode.h>

namespace opencos {

NameObj::NameObj(const NameObj &other) = default;

NameObj::NameObj(const Obj &obj)
    : Obj(obj.is_name() ? obj : Obj(nullptr))
{}

NameObj::~NameObj() = default;

std::string
NameObj::getName() const
{
    const CosString * const name_value = cos_name_obj_node_get_value(getNameImpl());
    if (name_value == nullptr) {
        return {};
    }

    return {cos_string_get_data(name_value),
            cos_string_get_length(name_value)};
}

NameObj::NameObj(CosNameObjNode *impl)
    : Obj(reinterpret_cast<CosObjNode *>(impl))
{}

CosNameObjNode *
NameObj::getNameImpl() const
{
    return reinterpret_cast<CosNameObjNode *>(Obj::getImpl());
}

} // namespace opencos

#endif /* LIBCOSXX_OBJECTS_NAME_OBJ_INL */
