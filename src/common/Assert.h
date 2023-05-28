//
// Created by david on 27/05/23.
//

#ifndef LIBCOS_ASSERT_H
#define LIBCOS_ASSERT_H

void
cos_assert_impl_(const char *condition,
                 const char *function_name,
                 const char *file,
                 int line,
                 const char *message,
                 ...);

#define COS_ASSERT(condition, message, ...)  \
    do {                                     \
        if ((condition) == false) {          \
            cos_assert_impl_(#condition,     \
                             __func__,       \
                             __FILE__,       \
                             __LINE__,       \
                             message,        \
                             ##__VA_ARGS__); \
        }                                    \
    } while (0)

#endif //LIBCOS_ASSERT_H
