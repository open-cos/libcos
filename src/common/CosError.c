//
// Created by david on 10/06/23.
//

#include "libcos/common/CosError.h"

#include <stdlib.h>

CosError *
cos_error_alloc(unsigned int code,
                char *message)
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
cos_error_free(CosError *error)
{
    if (error->message) {
        free(error->message);
    }

    free(error);
}

CosError *
cos_error_copy(CosError *error)
{
    if (!error) {
        return NULL;
    }

    return cos_error_alloc(error->code,
                           error->message);
}
