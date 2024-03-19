/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "common/CosString.h"

#include <libcos++/objects/NameObj.hpp>
#include <libcos/objects/CosNameObj.h>

namespace opencos {

NameObj::NameObj(const NameObj &other) = default;

NameObj::NameObj(const Obj &obj)
    : Obj(obj.is_name() ? obj : Obj(nullptr))
{}

NameObj::~NameObj() = default;

std::string
NameObj::getName() const
{
    const CosString * const name_value = cos_name_obj_get_value(getNameImpl());
    if (name_value == nullptr) {
        return {};
    }

    return {cos_string_get_data(name_value),
            cos_string_get_length(name_value)};
}

NameObj::NameObj(CosNameObj *impl)
    : Obj(reinterpret_cast<CosObj *>(impl))
{}

CosNameObj *
NameObj::getNameImpl() const
{
    return reinterpret_cast<CosNameObj *>(Obj::getImpl());
}

} // namespace opencos
