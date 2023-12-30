/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/common/CosLog.h"

#include "common/Assert.h"
#include "libcos/io/string-support.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

CosLogContext
cos_log_context_make(CosLogLevel level,
                     CosLogFunc log_func,
                     void * COS_Nullable user_data)
{
    const CosLogContext log_context = {
        .level = level,
        .log_func = log_func,
        .user_data = user_data,
    };
    return log_context;
}

void
cos_log(CosLogContext *log_context,
        CosLogMessageLevel message_level,
        const char *format,
        ...)
{
    COS_PARAMETER_ASSERT(log_context != NULL);

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
    if (!log_context || !log_context->log_func) {
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

COS_ASSUME_NONNULL_END
