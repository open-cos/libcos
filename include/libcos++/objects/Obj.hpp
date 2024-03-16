/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOSXX_OBJECTS_OBJ_HPP
#define LIBCOSXX_OBJECTS_OBJ_HPP

#include <libcos/common/CosTypes.h>

#include <memory>

namespace opencos {

class Obj {
public:
    Obj() = delete;
    Obj(const Obj &other);
    virtual ~Obj();

    explicit operator bool() const;

    explicit Obj(CosObj *impl);

    CosObj *
    getImpl() const;

private:
    std::shared_ptr<CosObj> impl_;
};

} // namespace opencos

#include <libcos++/impl/objects/Obj.inl>

#endif /* LIBCOSXX_OBJECTS_OBJ_HPP */
