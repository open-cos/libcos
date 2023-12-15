//
// Created by david on 10/06/23.
//

#include "libcos/common/CosError.h"

#include <stdlib.h>

CosError *
cos_error_alloc(CosErrorCode code, const char *message)
{
    CosError * const error = malloc(sizeof(CosError));
    if (!error) {
        return NULL;
    }

    error->code = code;
    error->message = message;

    return error;
}

void
cos_error_propagate(CosError *source_error, CosError **destination_error)
{
    if (!source_error || !destination_error) {
        return;
    }

    *destination_error = source_error;
}

CosError
cos_error_make(CosErrorCode code, const char *message)
{
    CosError result = {
        .code = code,
        .message = message,
    };
    return result;
}
