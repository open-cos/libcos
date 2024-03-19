/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOSXX_OBJECTS_ARRAY_OBJ_HPP
#define LIBCOSXX_OBJECTS_ARRAY_OBJ_HPP

#include <libcos++/objects/Obj.hpp>

#include <cstddef>
#include <vector>

namespace opencos {

class ArrayObj : public Obj {
public:
    ~ArrayObj() override;

    std::size_t
    getCount() const;

    Obj
    getAt(std::size_t index) const;

    explicit ArrayObj(CosArrayObj *impl);

    CosArrayObj *
    getArrayImpl() const;
};

} // namespace opencos

#include <libcos++/impl/objects/ArrayObj.inl>

#endif /* LIBCOSXX_OBJECTS_ARRAY_OBJ_HPP */
