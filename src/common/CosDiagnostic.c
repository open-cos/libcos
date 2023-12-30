/*
 * Copyright (c) 2023 OpenCOS.
 */

#include "libcos/common/CosDiagnostic.h"

#include "common/Assert.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

CosDiagnostic *
cos_diagnostic_alloc(CosDiagnosticType type,
                     const char *message)
{
    CosDiagnostic * const result = malloc(sizeof(CosDiagnostic));
    if (!result) {
        return NULL;
    }

    result->type = type;
    result->message = message;

    return result;
}

void
cos_diagnostic_free(CosDiagnostic *diagnostic)
{
    COS_PARAMETER_ASSERT(diagnostic != NULL);

    free(diagnostic);
}

COS_ASSUME_NONNULL_END
