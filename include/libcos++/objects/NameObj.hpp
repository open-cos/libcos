/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOSXX_OBJECTS_NAME_OBJ_HPP
#define LIBCOSXX_OBJECTS_NAME_OBJ_HPP

#include <libcos++/objects/Obj.hpp>

#include <string>

namespace opencos {

class NameObj : public Obj {
public:
    NameObj(const NameObj &other);
    explicit NameObj(const Obj &obj);
    ~NameObj() override;

    std::string
    getName() const;

    explicit NameObj(CosNameObj *impl);

    CosNameObj *
    getNameImpl() const;
};

} // namespace opencos

#include <libcos++/impl/objects/NameObj.inl>

#endif /* LIBCOSXX_OBJECTS_NAME_OBJ_HPP */
