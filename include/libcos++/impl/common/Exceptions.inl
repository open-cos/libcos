/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_IMPL_COMMON_EXCEPTIONS_INL
#define LIBCOS_IMPL_COMMON_EXCEPTIONS_INL

#include <libcos++/common/Exceptions.hpp>

#include <stdexcept>

namespace opencos {

void
error_to_exception(const CosError &error)
{
    switch (error.code) {
        case COS_ERROR_NONE:
            break;
        case COS_ERROR_INVALID_ARGUMENT:
            break;
        case COS_ERROR_INVALID_STATE:
            break;
        case COS_ERROR_OUT_OF_RANGE: {
            throw std::out_of_range(error.message);
        }
        case COS_ERROR_IO:
            break;
        case COS_ERROR_SYNTAX:
            break;
        case COS_ERROR_PARSE:
            break;
        case COS_ERROR_XREF:
            break;
        case COS_ERROR_MEMORY:
            break;
        case COS_ERROR_NOT_IMPLEMENTED:
            break;
        case COS_ERROR_UNKNOWN:
            break;
    }
}

} // namespace opencos

#endif /* LIBCOS_IMPL_COMMON_EXCEPTIONS_INL */
