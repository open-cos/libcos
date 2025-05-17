/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef CBIND_OWNERSHIP_OWNABLE_HPP
#define CBIND_OWNERSHIP_OWNABLE_HPP

#include <libcos++/common/unique.hpp>

#include <functional>
#include <memory>

namespace opencos {

template <typename T>
class Ownable {
public:
    explicit Ownable(T value)
        : value_(value), owned_(false), deleter_(nullptr) {}

    template <class Deleter>
    Ownable(T value, bool owned, Deleter deleter)
        : value_(value), owned_(owned), deleter_(deleter)
    {}

    Ownable(const Ownable &other) = delete;

    void
    operator=(const Ownable &other) = delete;

    ~Ownable()
    {
        if (owned_ && deleter_) {
            deleter_(value_);
        }
    }

    const T &
    value() const
    {
        return value_;
    }

    T &
    value()
    {
        return value_;
    }

private:
    T value_;
    bool owned_;
    std::function<void(T)> deleter_;
};

template <typename T>
std::unique_ptr<Ownable<T>>
make_unowned(T value)
{
    return make_unique<Ownable<T>>(value);
}

template <typename T, class Deleter>
std::unique_ptr<Ownable<T>>
make_owned(T value, Deleter deleter)
{
    return make_unique<Ownable<T>>(value, true, deleter);
}

template <typename T, class Deleter>
std::unique_ptr<Ownable<T>>
make_ownable(T value, bool owned, Deleter deleter)
{
    return make_unique<Ownable<T>>(value, owned, deleter);
}

} // namespace opencos

#endif /* CBIND_OWNERSHIP_OWNABLE_HPP */
