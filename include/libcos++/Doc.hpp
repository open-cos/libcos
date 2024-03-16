/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_DOC_HPP
#define LIBCOS_DOC_HPP

#include <libcos++/objects/Obj.hpp>
#include <libcos/common/CosTypes.h>

#include <memory>

namespace opencos {

class Doc {
public:
    Doc();
    ~Doc();

    Obj
    getRoot() const;

    explicit Doc(CosDoc *impl);

    CosDoc *
    getImpl() const;

private:
    std::shared_ptr<CosDoc> impl_;
};

} // namespace opencos

#include <libcos++/impl/Doc.inl>

#endif /* LIBCOS_DOC_HPP */
