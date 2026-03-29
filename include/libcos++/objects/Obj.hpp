/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOSXX_OBJECTS_OBJ_HPP
#define LIBCOSXX_OBJECTS_OBJ_HPP

#include <libcos++/ownership/Ownable.hpp>
#include <libcos/common/CosTypes.h>

#include <memory>

namespace opencos {

template <typename T>
using SharedOwnablePtr = std::shared_ptr<Ownable<T>>;

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

    explicit Obj(CosObjNode *impl, bool owner = false);

    CosObjNode *
    getImpl() const noexcept;

private:
    SharedOwnablePtr<CosObjNode *> impl_;
};

} // namespace opencos

#include <libcos++/impl/objects/Obj.inl>

#endif /* LIBCOSXX_OBJECTS_OBJ_HPP */
