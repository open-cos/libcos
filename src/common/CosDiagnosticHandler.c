/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/common/CosDiagnosticHandler.h"

#include "common/Assert.h"
#include "libcos/common/CosDiagnostic.h"
#include "libcos/common/CosLog.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

CosDiagnosticHandler
cos_diagnostic_handler_make(void (*handle_func)(CosDiagnosticHandler *handler,
                                                CosDiagnostic *diagnostic),
                            void * COS_Nullable user_data)
{
    const CosDiagnosticHandler result = {
        .handle_func = handle_func,
        .user_data = user_data,
    };
    return result;
}

void
cos_emit_diagnostic(CosDiagnosticHandler *handler,
                    CosDiagnostic *diagnostic)
{
    COS_PARAMETER_ASSERT(handler != NULL);
    COS_PARAMETER_ASSERT(diagnostic != NULL);
    if (!handler || !diagnostic) {
        return;
    }

    if (handler->handle_func != NULL) {
        handler->handle_func(handler, diagnostic);
    }
}

void
cos_diagnose(CosDiagnosticHandler *handler,
             CosDiagnosticType type,
             const char *message)
{
    COS_PARAMETER_ASSERT(handler != NULL);
    COS_PARAMETER_ASSERT(message != NULL);
    if (!handler || !message) {
        return;
    }

    CosDiagnostic diagnostic = {
        .type = type,
        .message = message,
    };
    cos_emit_diagnostic(handler, &diagnostic);
}

#pragma mark - Diagnostic logger

static void
cos_diagnostic_handler_log_(CosDiagnosticHandler *handler,
                            CosDiagnostic *diagnostic)
{
    COS_PARAMETER_ASSERT(handler != NULL);
    COS_PARAMETER_ASSERT(diagnostic != NULL);
    if (!handler || !diagnostic) {
        return;
    }

    CosLogContext * const log_context = handler->user_data;
    if (!log_context) {
        return;
    }

    CosLogMessageLevel message_level;
    switch (diagnostic->type) {
        case CosDiagnosticLevel_Warning:
            message_level = CosLogMessageLevel_Warning;
            break;
        case CosDiagnosticLevel_Error:
            message_level = CosLogMessageLevel_Error;
            break;
    }

    cos_log(log_context, message_level, "Error");
}

CosDiagnosticHandler
cos_diagnostic_handler_make_logger(CosLogContext *log_context)
{
    return cos_diagnostic_handler_make(cos_diagnostic_handler_log_,
                                       log_context);
}

COS_ASSUME_NONNULL_END
