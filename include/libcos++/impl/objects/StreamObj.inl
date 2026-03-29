/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOSXX_OBJECTS_STREAM_OBJ_INL
#define LIBCOSXX_OBJECTS_STREAM_OBJ_INL

#include "libcos/common/CosArray.h"
#include "libcos/common/CosError.h"
#include "libcos/objects/CosArrayObjNode.h"
#include "objects/ArrayObj.hpp"
#include "objects/NameObj.hpp"

#include <libcos++/objects/StreamObj.hpp>

#include <stdexcept>

namespace opencos {

StreamObj::~StreamObj() = default;

std::size_t
StreamObj::getLength() const
{
    return cos_stream_obj_node_get_length(getStreamImpl());
}

std::vector<std::string>
StreamObj::getFilterNames() const
{
    std::vector<std::string> filterNames;

    CosError error;
    CosArrayObjNode * const filter_names = cos_stream_obj_node_get_filter_names(getStreamImpl(),
                                                                       &error);
    if (filter_names == nullptr) {
        return filterNames;
    }

    ArrayObj filterNamesArray(filter_names);

    const std::size_t count = filterNamesArray.getCount();
    for (std::size_t i = 0; i < count; ++i) {
        const Obj filterNameObj = filterNamesArray.getAt(i);

        if (auto filterName = NameObj(filterNameObj)) {
            filterNames.push_back(filterName.getName());
        }
    }

    return filterNames;
}

StreamObj::StreamObj(CosStreamObjNode *impl)
    : Obj(reinterpret_cast<CosObjNode *>(impl))
{
}

CosStreamObjNode *
StreamObj::getStreamImpl() const noexcept
{
    return reinterpret_cast<CosStreamObjNode *>(Obj::getImpl());
}

} // namespace opencos

#endif /* LIBCOSXX_OBJECTS_STREAM_OBJ_INL */
