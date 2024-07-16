/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/common/CosDiagnosticHandler.h"

#include "common/Assert.h"
#include "libcos/common/CosDiagnostic.h"
#include "libcos/common/CosLog.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosDiagnosticHandler {
    CosDiagnosticHandlerFunc handle_func;

    void * COS_Nullable user_data;
};

static void
cos_diagnostic_handler_default_log_(CosDiagnosticHandler *handler,
                                    const CosDiagnostic *diagnostic)
{
    COS_PARAMETER_ASSERT(handler != NULL);
    COS_PARAMETER_ASSERT(diagnostic != NULL);
    if (!handler || !diagnostic) {
        return;
    }

    const char *diagnostic_type_name = NULL;
    switch (diagnostic->type) {
        case CosDiagnosticLevel_Warning:
            diagnostic_type_name = "warning";
            break;
        case CosDiagnosticLevel_Error:
            diagnostic_type_name = "error";
            break;
    }

    printf("%s: %s\n", diagnostic_type_name, diagnostic->message);
}

static CosDiagnosticHandler cos_diagnostic_handler_default_ = {
    .handle_func = &cos_diagnostic_handler_default_log_,
    .user_data = NULL,
};

CosDiagnosticHandler *
cos_diagnostic_handler_alloc(CosDiagnosticHandlerFunc handle_func,
                             void * COS_Nullable user_data)
{
    CosDiagnosticHandler *handler = calloc(1, sizeof(CosDiagnosticHandler));
    if (!handler) {
        return NULL;
    }

    handler->handle_func = handle_func;
    handler->user_data = user_data;

    return handler;
}

void
cos_diagnostic_handler_free(CosDiagnosticHandler *handler)
{
    if (!handler) {
        return;
    }

    free(handler);
}

CosDiagnosticHandler *
cos_diagnostic_handler_get_default(void)
{
    return &cos_diagnostic_handler_default_;
}

void *
cos_diagnostic_handler_get_user_data(const CosDiagnosticHandler *handler)
{
    COS_PARAMETER_ASSERT(handler != NULL);
    if (!handler) {
        return NULL;
    }

    return handler->user_data;
}

void
cos_emit_diagnostic(CosDiagnosticHandler *handler,
                    const CosDiagnostic *diagnostic)
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

    const CosDiagnostic diagnostic = {
        .type = type,
        .message = message,
    };
    cos_emit_diagnostic(handler, &diagnostic);
}

// MARK: - Diagnostic logger

static void
cos_diagnostic_handler_log_(CosDiagnosticHandler *handler,
                            const CosDiagnostic *diagnostic)
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

CosDiagnosticHandler *
cos_diagnostic_handler_alloc_logger(CosLogContext *log_context)
{
    return cos_diagnostic_handler_alloc(&cos_diagnostic_handler_log_,
                                        log_context);
}

COS_ASSUME_NONNULL_END
