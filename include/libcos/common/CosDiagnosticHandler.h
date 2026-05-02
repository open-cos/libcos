/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COS_DIAGNOSTIC_HANDLER_H
#define LIBCOS_COS_DIAGNOSTIC_HANDLER_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosDiagnostic.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef void (*CosDiagnosticHandlerFunc)(CosDiagnosticHandler *handler,
                                         const CosDiagnostic *diagnostic);

COS_API CosDiagnosticHandler * COS_Nullable
cos_diagnostic_handler_alloc(CosDiagnosticHandlerFunc handle_func,
                             void * COS_Nullable user_data)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

COS_API void
cos_diagnostic_handler_free(CosDiagnosticHandler *handler);

/**
 * @brief Returns the default diagnostic handler.
 *
 * @return The default diagnostic handler.
 */
COS_API CosDiagnosticHandler *
cos_diagnostic_handler_get_default(void);

COS_API void * COS_Nullable
cos_diagnostic_handler_get_user_data(const CosDiagnosticHandler *handler);

COS_API void
cos_emit_diagnostic(CosDiagnosticHandler *handler,
                    const CosDiagnostic *diagnostic);

COS_API void
cos_diagnose(CosDiagnosticHandler *handler,
             CosDiagnosticType type,
             const char *message);

// MARK: - Diagnostic logger

COS_API CosDiagnosticHandler * COS_Nullable
cos_diagnostic_handler_alloc_logger(CosLogContext *log_context)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_DIAGNOSTIC_HANDLER_H */
