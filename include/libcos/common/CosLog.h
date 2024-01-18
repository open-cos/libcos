/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COS_LOG_H
#define LIBCOS_COS_LOG_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdarg.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef enum CosLogLevel {
    CosLogLevel_None = 0,
    CosLogLevel_Fatal,
    CosLogLevel_Error,
    CosLogLevel_Warning,
    CosLogLevel_Info,
    CosLogLevel_Debug,
} CosLogLevel;

typedef enum CosLogMessageLevel {
    CosLogMessageLevel_Fatal = CosLogLevel_Fatal,
    CosLogMessageLevel_Error = CosLogLevel_Error,
    CosLogMessageLevel_Warning = CosLogLevel_Warning,
    CosLogMessageLevel_Info = CosLogLevel_Info,
    CosLogMessageLevel_Debug = CosLogLevel_Debug,
} CosLogMessageLevel;

typedef void (*CosLogFunc)(CosLogContext *log_context,
                           CosLogMessageLevel message_level,
                           const char *message);

/**
 * @brief Allocates a new log context.
 *
 * @param level The logging level.
 * @param log_func The logging function.
 * @param user_data Optional user data.
 *
 * @return A new log context, or @c NULL if allocation failed.
 */
CosLogContext * COS_Nullable
cos_log_context_make(CosLogLevel level,
                     CosLogFunc log_func,
                     void * COS_Nullable user_data)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_log_context_free(CosLogContext *log_context);

void
cos_log(CosLogContext *log_context,
        CosLogMessageLevel message_level,
        const char *format,
        ...)
    COS_FORMAT_PRINTF(3, 4);

void
cos_logv(CosLogContext *log_context,
         CosLogMessageLevel message_level,
         const char *format,
         va_list args)
    COS_FORMAT_PRINTF(3, 0);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_LOG_H */
