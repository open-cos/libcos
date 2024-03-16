/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_DOC_INL
#define LIBCOS_DOC_INL

#include <libcos++/Doc.hpp>
#include <libcos/CosDoc.h>

#include <new>

namespace opencos {

namespace detail {

struct DocDeleter {
    void
    operator()(CosDoc *doc) const
    {
        cos_doc_destroy(doc);
    }
};

} // namespace detail

Doc::Doc()
{
    CosDoc * const doc = cos_doc_create(nullptr);
    if (doc == nullptr) {
        throw std::bad_alloc();
    }
    impl_ = std::shared_ptr<CosDoc>(doc, detail::DocDeleter());
}

Doc::~Doc() = default;

Obj
Doc::getRoot() const
{
    return Obj(cos_doc_get_root(impl_.get()));
}

Doc::Doc(CosDoc *impl)
    : impl_(impl, detail::DocDeleter())
{}

CosDoc *
Doc::getImpl() const
{
    return impl_.get();
}

} // namespace opencos

#endif /* LIBCOS_DOC_INL */
