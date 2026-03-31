/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOSXX_OBJECTS_OBJ_INL
#define LIBCOSXX_OBJECTS_OBJ_INL

#include <libcos++/objects/Obj.hpp>
#include <libcos/objects/CosObjNode.h>

namespace opencos {

namespace detail {

struct ObjDeleter {
    void
    operator()(CosObjNode *obj) const
    {
        cos_obj_node_release(obj);
    }
};

} // namespace detail

Obj::Obj() = default;

Obj::~Obj() = default;

Obj::
operator bool() const
{
    return impl_ && (impl_->value() != nullptr);
}

bool
Obj::is_name() const noexcept
{
    return cos_obj_node_is_name(getImpl());
}

Obj::Obj(CosObjNode *impl, bool owner)
    : impl_(make_ownable(impl,
                         owner,
                         detail::ObjDeleter()))
{}

CosObjNode *
Obj::getImpl() const noexcept
{
    return impl_->value();
}

} // namespace opencos

#endif /* LIBCOSXX_OBJECTS_OBJ_INL */
