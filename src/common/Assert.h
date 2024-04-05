/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_ASSERT_H
#define LIBCOS_ASSERT_H

#include <libcos/common/CosDefines.h>

#if !defined(COS_DISABLE_ASSERTIONS)

#define COS_ASSERT(condition, ...) \
    COS_ASSERT_(condition, #condition, __VA_ARGS__)

#define COS_PARAMETER_ASSERT(condition) \
    COS_ASSERT_(condition, #condition, "invalid parameter")

#define COS_PARAM_ASSERT_INTERNAL(condition) \
    COS_ASSERT_FATAL_(condition, #condition, "invalid internal parameter")

#else

#define COS_ASSERT(condition, ...) ((void)0)

#define COS_PARAMETER_ASSERT(condition) ((void)0)

#define COS_PARAM_ASSERT_INTERNAL(condition) ((void)0)

#endif

#define COS_ASSERT_(condition, condition_msg, ...) \
    do {                                           \
        if (COS_UNLIKELY(!(condition))) {          \
            cos_assertion_failure_(condition_msg,  \
                                   __func__,       \
                                   __FILE__,       \
                                   __LINE__,       \
                                   __VA_ARGS__);   \
        }                                          \
    } while (0)

#define COS_ASSERT_FATAL_(condition, condition_msg, ...) \
    do {                                                 \
        if (COS_UNLIKELY(!(condition))) {                \
            cos_fatal_assertion_failure_(condition_msg,  \
                                         __func__,       \
                                         __FILE__,       \
                                         __LINE__,       \
                                         __VA_ARGS__);   \
        }                                                \
    } while (0)

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

void
cos_assertion_failure_(const char *condition,
                       const char *function_name,
                       const char *file,
                       int line,
                       const char * COS_Nullable message,
                       ...)
    COS_FORMAT_PRINTF(5, 6);

void
cos_fatal_assertion_failure_(const char *condition,
                             const char *function_name,
                             const char *file,
                             int line,
                             const char * COS_Nullable message,
                             ...)
    COS_FORMAT_PRINTF(5, 6)
    COS_NORETURN;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif //LIBCOS_ASSERT_H
