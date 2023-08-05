//
// Created by david on 10/06/23.
//

#ifndef LIBCOS_COS_ERROR_H
#define LIBCOS_COS_ERROR_H

struct CosError;
typedef struct CosError CosError;

enum CosErrorCode {
    COS_ERROR_NONE,
    COS_ERROR_INVALID_ARGUMENT,
    COS_ERROR_INVALID_STATE,
    COS_ERROR_IO,
    COS_ERROR_PARSE,
    COS_ERROR_MEMORY,
    COS_ERROR_NOT_IMPLEMENTED,
    COS_ERROR_UNKNOWN,
};

struct CosError {
    unsigned int code;
    char *message;
};

CosError *
cos_error_alloc(unsigned int code,
                char *message);

void
cos_error_free(CosError *error);

CosError *
cos_error_copy(CosError *error);

void
cos_error_propagate(CosError *source_error,
                    CosError **destination_error);

#endif /* LIBCOS_COS_ERROR_H */
