//
// Created by david on 10/06/23.
//

#ifndef LIBCOS_COS_ERROR_H
#define LIBCOS_COS_ERROR_H

struct CosError;
typedef struct CosError CosError;

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

#endif /* LIBCOS_COS_ERROR_H */
