/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_COS_TRAILER_PRIVATE_H
#define LIBCOS_COS_TRAILER_PRIVATE_H

#include <libcos/CosTrailer.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * Links @p prev as the previous (older) revision of @p trailer.
 *
 * @param trailer The newer trailer.
 * @param prev The older trailer (ownership transferred; destroyed with @p trailer).
 */
void
cos_trailer_set_prev_(CosTrailer *trailer,
                      CosTrailer * COS_Nullable prev)
    COS_OWNERSHIP_TAKES(2);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_TRAILER_PRIVATE_H */
