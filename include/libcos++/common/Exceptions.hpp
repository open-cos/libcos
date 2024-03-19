/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOSXX_COMMON_EXCEPTIONS_HPP
#define LIBCOSXX_COMMON_EXCEPTIONS_HPP

#include <libcos/common/CosError.h>
#include <libcos/common/CosTypes.h>

#include <exception>

namespace opencos {

inline void
error_to_exception(const CosError &error);

} // namespace opencos

#include <libcos++/impl/common/Exceptions.inl>

#endif /* LIBCOSXX_COMMON_EXCEPTIONS_HPP */
