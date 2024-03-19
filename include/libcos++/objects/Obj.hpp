/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOSXX_OBJECTS_OBJ_HPP
#define LIBCOSXX_OBJECTS_OBJ_HPP

#include <cbind/ownership/Ownable.hpp>
#include <libcos/common/CosTypes.h>

#include <memory>

namespace opencos {

template <typename T>
using SharedOwnablePtr = std::shared_ptr<cbind::Ownable<T>>;

class Obj {
public:
    Obj();
    virtual ~Obj();

    Obj(const Obj &other) = default;
    Obj &
    operator=(const Obj &other) = default;

    explicit
    operator bool() const;

    bool
    is_name() const noexcept;

    explicit Obj(CosObj *impl, bool owner = false);

    CosObj *
    getImpl() const noexcept;

private:
    SharedOwnablePtr<CosObj *> impl_;
};

} // namespace opencos

#include <libcos++/impl/objects/Obj.inl>

#endif /* LIBCOSXX_OBJECTS_OBJ_HPP */
