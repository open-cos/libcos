/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COS_DIAGNOSTIC_HANDLER_H
#define LIBCOS_COS_DIAGNOSTIC_HANDLER_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosDiagnostic.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosDiagnosticHandler {
    void (*handle_func)(CosDiagnosticHandler *handler,
                        CosDiagnostic *diagnostic);

    void * COS_Nullable user_data;
};

CosDiagnosticHandler
cos_diagnostic_handler_make(void (*handle_func)(CosDiagnosticHandler *handler,
                                                CosDiagnostic *diagnostic),
                            void * COS_Nullable user_data);

void
cos_emit_diagnostic(CosDiagnosticHandler *handler,
                    CosDiagnostic *diagnostic);

void
cos_diagnose(CosDiagnosticHandler *handler,
             CosDiagnosticType type,
             const char *message);

#pragma mark - Diagnostic logger

CosDiagnosticHandler
cos_diagnostic_handler_make_logger(CosLogContext *log_context);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_DIAGNOSTIC_HANDLER_H */
