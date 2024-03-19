/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_UNIQUE_HPP
#define LIBCOS_COMMON_UNIQUE_HPP

#include <type_traits>

#include <memory>
#include <utility>

namespace opencos {
namespace cbind {

namespace detail {

template <typename T,
          typename... Args>
std::unique_ptr<T>
make_unique_helper(std::false_type,
                   Args &&...args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T,
          typename... Args>
std::unique_ptr<T>
make_unique_helper(std::true_type,
                   Args &&...args)
{
    static_assert(std::extent<T>::value == 0,
                  "make_unique<T[N]>() is forbidden, please use make_unique<T[]>().");

    using U = typename std::remove_extent<T>::type;
    return std::unique_ptr<T>(new U[sizeof...(Args)]{std::forward<Args>(args)...});
}

} // namespace detail

template <typename T,
          typename... Args>
std::unique_ptr<T>
make_unique(Args &&...args)
{
    return detail::make_unique_helper<T>(std::is_array<T>(),
                                         std::forward<Args>(args)...);
}

} // namespace cbind
} // namespace opencos

#endif /* LIBCOS_COMMON_UNIQUE_HPP */
