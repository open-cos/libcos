/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/common/CosLog.h"

#include "common/Assert.h"

#include "libcos/io/string-support.h"

#include <stdio.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosLogContext {
    /**
     * @brief The logging level.
     *
     * @note Messages with a level above this level will be filtered out.
     */
    CosLogLevel level;

    /**
     * @brief The logging function.
     */
    CosLogFunc log_func;

    /**
     * @brief Optional user data.
     */
    void * COS_Nullable user_data;
};

static void
cos_log_func_stdout_(CosLogContext *log_context,
                     CosLogMessageLevel message_level,
                     const char *message);

static CosLogContext default_log_context = {
    .level = CosLogLevel_Info,
    .log_func = &cos_log_func_stdout_,
    .user_data = NULL,
};

CosLogContext *
cos_log_context_get_default(void)
{
    return &default_log_context;
}

CosLogContext *
cos_log_context_create(CosLogLevel level,
                       CosLogFunc log_func,
                       void * COS_Nullable user_data)
{
    COS_PARAMETER_ASSERT(log_func != NULL);
    if (!log_func) {
        return NULL;
    }

    CosLogContext *log_context = calloc(1, sizeof(CosLogContext));
    if (!log_context) {
        return NULL;
    }

    log_context->level = level;
    log_context->log_func = log_func;
    log_context->user_data = user_data;

    return log_context;
}

void
cos_log_context_destroy(CosLogContext *log_context)
{
    COS_PARAMETER_ASSERT(log_context != NULL);
    if (!log_context) {
        return;
    }

    free(log_context);
}

CosLogLevel
cos_log_context_get_level(const CosLogContext *log_context)
{
    COS_PARAMETER_ASSERT(log_context != NULL);
    if (!log_context) {
        return CosLogLevel_None;
    }

    return log_context->level;
}

void
cos_log_context_set_level(CosLogContext *log_context,
                          CosLogLevel level)
{
    COS_PARAMETER_ASSERT(log_context != NULL);
    if (!log_context) {
        return;
    }

    log_context->level = level;
}

void
cos_log(CosLogContext *log_context,
        CosLogMessageLevel message_level,
        const char *format,
        ...)
{
    COS_PARAMETER_ASSERT(log_context != NULL);
    COS_PARAMETER_ASSERT(format != NULL);
    if (COS_UNLIKELY(!log_context || !format)) {
        return;
    }

    va_list args;
    va_start(args, format);
    cos_logv(log_context, message_level, format, args);
    va_end(args);
}

void
cos_logv(CosLogContext *log_context,
         CosLogMessageLevel message_level,
         const char *format,
         va_list args)
{
    COS_PARAMETER_ASSERT(log_context != NULL);
    COS_PARAMETER_ASSERT(format != NULL);
    if (COS_UNLIKELY(!log_context || !format || !log_context->log_func)) {
        return;
    }

    // Filter out messages that are above the logging level.
    if ((int)(log_context->level) < (int)message_level) {
        return;
    }

    // Format the message.
    char * const message = cos_vasprintf(format, args);
    if (!message) {
        return;
    }

    log_context->log_func(log_context, message_level, message);

    free(message);
}

static void
cos_log_func_stdout_(CosLogContext *log_context,
                     CosLogMessageLevel message_level,
                     const char *message)
{
    COS_PARAMETER_ASSERT(log_context != NULL);
    COS_PARAMETER_ASSERT(message != NULL);
    if (COS_UNLIKELY(!log_context || !message)) {
        return;
    }

    // Get the string representation of the message level.
    const char *level_str = NULL;
    switch (message_level) {
        case CosLogMessageLevel_Fatal:
            level_str = "FATAL";
            break;
        case CosLogMessageLevel_Error:
            level_str = "ERROR";
            break;
        case CosLogMessageLevel_Warning:
            level_str = "WARNING";
            break;
        case CosLogMessageLevel_Info:
            level_str = "INFO";
            break;
        case CosLogMessageLevel_Trace:
            level_str = "TRACE";
            break;
    }
    if (COS_UNLIKELY(!level_str)) {
        level_str = "UNKNOWN";
    }

    (void)fprintf(stdout,
                  "[%s] %s\n",
                  level_str,
                  message);
}

COS_ASSUME_NONNULL_END
