//
// Created by david on 10/06/23.
//

#include "libcos/common/CosError.h"

#include "common/Assert.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

const CosError CosErrorNone = {
    .code = COS_ERROR_NONE,
    .message = NULL,
};

CosError
cos_error_none(void)
{
    const CosError result = {
        .code = COS_ERROR_NONE,
        .message = NULL,
    };
    return result;
}

void
cos_error_propagate(CosError * COS_Nullable destination_error,
                    CosError source_error)
{
    COS_PARAMETER_ASSERT(destination_error != NULL);
    if (!destination_error) {
        return;
    }

    *destination_error = source_error;
}

CosError
cos_error_make(CosErrorCode code,
               const char *message)
{
    CosError result = {
        .code = code,
        .message = message,
    };
    return result;
}

CosError
cos_error_make_invalid_argument(const char *message)
{
    return cos_error_make(COS_ERROR_INVALID_ARGUMENT, message);
}

void
cos_error_propagate_(CosError source_error,
                     CosError * COS_Nullable destination_error)
{
    if (destination_error) {
        *destination_error = source_error;
    }
}

COS_ASSUME_NONNULL_END
