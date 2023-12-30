/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COS_DIAGNOSTIC_H
#define LIBCOS_COS_DIAGNOSTIC_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef enum CosDiagnosticType {
    CosDiagnosticLevel_Warning,
    CosDiagnosticLevel_Error,
} CosDiagnosticType;

struct CosDiagnostic {
    /**
     * @brief The diagnostic type.
     */
    CosDiagnosticType type;

    /**
     * @brief The diagnostic message.
     */
    const char *message;
};

CosDiagnostic * COS_Nullable
cos_diagnostic_alloc(CosDiagnosticType type,
                     const char *message)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_diagnostic_free(CosDiagnostic *diagnostic);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_DIAGNOSTIC_H */
