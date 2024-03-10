//
// Created by david on 27/05/23.
//

#ifndef LIBCOS_ASSERT_H
#define LIBCOS_ASSERT_H

#include <libcos/common/CosDefines.h>

#define COS_ASSERT(condition, ...)         \
    do {                                   \
        if (COS_UNLIKELY(!(condition))) {  \
            cos_assert_impl_(#condition,   \
                             __func__,     \
                             __FILE__,     \
                             __LINE__,     \
                             __VA_ARGS__); \
        }                                  \
    } while (0)

#define COS_PARAMETER_ASSERT(condition)            \
    do {                                           \
        if (COS_UNLIKELY(!(condition))) {          \
            cos_assert_impl_(#condition,           \
                             __func__,             \
                             __FILE__,             \
                             __LINE__,             \
                             "invalid parameter"); \
        }                                          \
    } while (0)

void
cos_assert_impl_(const char *condition,
                 const char *function_name,
                 const char *file,
                 int line,
                 const char *message,
                 ...)
    COS_FORMAT_PRINTF(5, 6);

#endif //LIBCOS_ASSERT_H
