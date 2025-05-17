/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOSXX_OBJECTS_OBJ_INL
#define LIBCOSXX_OBJECTS_OBJ_INL

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

Obj::Obj() = default;

Obj::~Obj() = default;

Obj::operator bool() const
{
    return impl_ && (impl_->value() != nullptr);
}

bool
Obj::is_name() const noexcept
{
    return cos_obj_is_name(getImpl());
}

Obj::Obj(CosObj *impl, bool owner)
    : impl_(cbind::make_ownable(impl,
                                owner,
                                detail::ObjDeleter()))
{}

CosObj *
Obj::getImpl() const noexcept
{
    return impl_->value();
}

} // namespace opencos

#endif /* LIBCOSXX_OBJECTS_OBJ_INL */
