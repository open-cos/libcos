/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOSXX_OBJECTS_STREAM_OBJ_HPP
#define LIBCOSXX_OBJECTS_STREAM_OBJ_HPP

#include <libcos++/objects/Obj.hpp>
#include <libcos/objects/CosStreamObj.h>

#include <cstddef>

namespace opencos {

class StreamObj : public Obj {
public:
    ~StreamObj() override;

    std::size_t
    getLength() const;

    explicit StreamObj(CosStreamObj *impl);

    CosStreamObj *
    getStreamImpl() const;
};

} // namespace opencos

#include <libcos++/impl/objects/StreamObj.inl>

#endif /* LIBCOSXX_OBJECTS_STREAM_OBJ_HPP */
