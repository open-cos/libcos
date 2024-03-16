/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOSXX_OBJECTS_STREAM_OBJ_INL
#define LIBCOSXX_OBJECTS_STREAM_OBJ_INL

#include <libcos++/objects/StreamObj.hpp>

namespace opencos {

StreamObj::~StreamObj() = default;

StreamObj::StreamObj(CosStreamObj *impl)
    : Obj(reinterpret_cast<CosObj *>(impl))
{
}

std::size_t
StreamObj::getLength() const
{
    return cos_stream_obj_get_length(getStreamImpl());
}

CosStreamObj *
StreamObj::getStreamImpl() const
{
    return reinterpret_cast<CosStreamObj *>(Obj::getImpl());
}

} // namespace opencos

#endif /* LIBCOSXX_OBJECTS_STREAM_OBJ_INL */
