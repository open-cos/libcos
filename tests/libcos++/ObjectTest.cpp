/*
 * Copyright (c) 2024 OpenCOS.
 */

#include <libcos++/Doc.hpp>
#include <libcos++/objects/StreamObj.hpp>

namespace opencos {
namespace tests {

class StreamObjTest {
public:
    static void testCreate() {
        opencos::Doc doc;

        doc.getImpl();

        StreamObj stream_obj(nullptr);

        stream_obj.getImpl();
    }
};

} // namespace tests
} // namespace opencos

