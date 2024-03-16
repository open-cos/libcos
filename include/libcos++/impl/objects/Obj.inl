/*
 * Copyright (c) 2024 OpenCOS.
 */

#include <libcos++/objects/Obj.hpp>
#include <libcos/objects/CosObj.h>

namespace opencos {

namespace detail {

struct ObjDeleter {
    void
    operator()(CosObj *obj) const
    {
        cos_obj_free(obj);
    }
};

} // namespace detail

Obj::Obj(const Obj &other) = default;

Obj::~Obj() = default;

Obj::operator bool() const
{
    return impl_ != nullptr;
}

Obj::Obj(CosObj *impl)
    : impl_(impl, detail::ObjDeleter())
{}

CosObj *
Obj::getImpl() const
{
    return impl_.get();
}

} // namespace opencos
