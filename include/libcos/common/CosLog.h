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
    CosLogLevel_Trace,
} CosLogLevel;

typedef enum CosLogMessageLevel {
    CosLogMessageLevel_Fatal = CosLogLevel_Fatal,
    CosLogMessageLevel_Error = CosLogLevel_Error,
    CosLogMessageLevel_Warning = CosLogLevel_Warning,
    CosLogMessageLevel_Info = CosLogLevel_Info,
    CosLogMessageLevel_Trace = CosLogLevel_Trace,
} CosLogMessageLevel;

typedef void (*CosLogFunc)(CosLogContext *log_context,
                           CosLogMessageLevel message_level,
                           const char *message);

/**
 * @brief The default log context.
 *
 * @return The default log context.
 */
CosLogContext *
cos_log_context_get_default(void);

/**
 * @brief Destroys a log context.
 *
 * @param log_context The log context.
 */
void
cos_log_context_destroy(CosLogContext *log_context)
    COS_DEALLOCATOR_FUNC;

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
cos_log_context_create(CosLogLevel level,
                       CosLogFunc log_func,
                       void * COS_Nullable user_data)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_log_context_destroy);

/**
 * @brief Gets the logging level of a log context.
 *
 * @param log_context The log context.
 *
 * @return The logging level.
 */
CosLogLevel
cos_log_context_get_level(const CosLogContext *log_context);

/**
 * @brief Sets the logging level of a log context.
 *
 * @param log_context The log context.
 * @param level The logging level.
 */
void
cos_log_context_set_level(CosLogContext *log_context,
                          CosLogLevel level);

/**
 * @brief Logs a message with a given log context.
 *
 * @param log_context The log context.
 * @param message_level The level of the message.
 * @param format The <tt>printf</tt>-style format string.
 * @param ... The format string arguments.
 */
void
cos_log(CosLogContext *log_context,
        CosLogMessageLevel message_level,
        const char *format,
        ...)
    COS_FORMAT_PRINTF(3, 4);

/**
 * @brief Logs a message with a given log context.
 *
 * @param log_context The log context.
 * @param message_level The level of the message.
 * @param format The <tt>printf</tt>-style format string.
 * @param args The format string arguments.
 */
void
cos_logv(CosLogContext *log_context,
         CosLogMessageLevel message_level,
         const char *format,
         va_list args)
    COS_FORMAT_PRINTF(3, 0);

#define COS_LOG_FATAL(log_context, ...) \
    cos_log(log_context, CosLogMessageLevel_Fatal, __VA_ARGS__)

#define COS_LOG_ERROR(log_context, ...) \
    cos_log(log_context, CosLogMessageLevel_Error, __VA_ARGS__)

#define COS_LOG_WARNING(log_context, ...) \
    cos_log(log_context, CosLogMessageLevel_Warning, __VA_ARGS__)

#define COS_LOG_INFO(log_context, ...) \
    cos_log(log_context, CosLogMessageLevel_Info, __VA_ARGS__)

#define COS_LOG_TRACE(log_context, ...) \
    cos_log(log_context, CosLogMessageLevel_Trace, __VA_ARGS__)

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_LOG_H */
