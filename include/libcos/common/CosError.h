//
// Created by david on 10/06/23.
//

#ifndef LIBCOS_COS_ERROR_H
#define LIBCOS_COS_ERROR_H

struct CosError;
typedef struct CosError CosError;

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

CosError
cos_error_make(CosErrorCode code, const char *message);

CosError *
cos_error_alloc(CosErrorCode code, const char *message);

void
cos_error_propagate(CosError *source_error, CosError **destination_error);

#endif /* LIBCOS_COS_ERROR_H */
