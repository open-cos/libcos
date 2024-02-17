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
    COS_ERROR_OUT_OF_RANGE,
    COS_ERROR_IO,
    COS_ERROR_SYNTAX,
    COS_ERROR_PARSE,
    COS_ERROR_XREF,
    COS_ERROR_MEMORY,
    COS_ERROR_NOT_IMPLEMENTED,
    COS_ERROR_UNKNOWN,
} CosErrorCode;

struct CosError {
    CosErrorCode code;
    const char * COS_Nullable message;
};

/**
 * The default empty error value.
 */
extern const CosError CosErrorNone;

#define COS_ERROR_PROPAGATE(source_error, destination_error) \
    cos_error_propagate_(source_error, destination_error)

CosError
cos_error_none(void)
    COS_ATTR_PURE;

CosError
cos_error_make(CosErrorCode code,
               const char *message);

/**
 * @brief Propagates the source error to the destination error.
 *
 * @param destination_error The destination error.
 * @param source_error The source error.
 */
void
cos_error_propagate(CosError * COS_Nullable destination_error,
                    CosError source_error)
    COS_ATTR_ACCESS_WRITE_ONLY(1);

void
cos_error_propagate_(CosError source_error,
                     CosError * COS_Nullable destination_error)
    COS_ATTR_ACCESS_WRITE_ONLY(2);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_ERROR_H */
