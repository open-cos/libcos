//
// Created by david on 10/06/23.
//

#ifndef LIBCOS_COS_ERROR_H
#define LIBCOS_COS_ERROR_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef enum CosErrorCode {
    COS_ERROR_NONE,
    COS_ERROR_INVALID_ARGUMENT,
    COS_ERROR_INVALID_STATE,
    COS_ERROR_IO,
    COS_ERROR_SYNTAX,
    COS_ERROR_PARSE,
    COS_ERROR_MEMORY,
    COS_ERROR_NOT_IMPLEMENTED,
    COS_ERROR_UNKNOWN,
} CosErrorCode;

struct CosError {
    CosErrorCode code;
    const char *message;
};

#define COS_ERROR_PROPAGATE(source_error, destination_error) \
    cos_error_propagate_(source_error, destination_error)

CosError
cos_error_make(CosErrorCode code, const char *message);

CosError * COS_Nullable
cos_error_alloc(CosErrorCode code, const char *message);

void
cos_error_propagate(CosError *source_error, CosError **destination_error);

void
cos_error_propagate_(CosError source_error,
                     CosError *destination_error)
    COS_ATTR_ACCESS_WRITE_ONLY(2);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_ERROR_H */
