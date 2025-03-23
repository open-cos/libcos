/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_API_H
#define LIBCOS_COMMON_COS_API_H

#include <libcos/common/CosDefines.h>

// MARK: - Clang Bounds Safety

#if COS_HAS_FEATURE(bounds_safety)
    #define COS_HAS_BOUNDS_SAFETY 1
#else
    #define COS_HAS_BOUNDS_SAFETY 0
#endif

// MARK: - Microsoft SAL Annotations

#if defined(_MSC_VER) || COS_HAS_INCLUDE(<sal.h>)
    #define COS_HAS_SAL 1
    #include <sal.h>
#else
    #define COS_HAS_SAL 0
#endif

#if COS_HAS_BOUNDS_SAFETY

    #define COS_ATTR_in_nonnull_counted_by(x) __counted_by(x)
    #define COS_ATTR_in_nullable_counted_by(x) __counted_by_or_null(x)

    #define COS_ATTR_in_nonnull_sized_by(x) __sized_by(x)
    #define COS_ATTR_in_nullable_sized_by(x) __sized_by_or_null(x)

    #define COS_ATTR_out_nonnull_counted_by(x) __counted_by(x)
    #define COS_ATTR_out_nullable_counted_by(x) __counted_by_or_null(x)

    #define COS_ATTR_out_nonnull_sized_by(x) __sized_by(x)
    #define COS_ATTR_out_nullable_sized_by(x) __sized_by_or_null(x)

#elif COS_HAS_SAL

    #define COS_ATTR_in_nonnull_counted_by(x) _In_reads_(x)
    #define COS_ATTR_in_nullable_counted_by(x) _In_reads_opt_(x)

    #define COS_ATTR_in_nonnull_sized_by(x) _In_reads_bytes_(x)
    #define COS_ATTR_in_nullable_sized_by(x) _In_reads_bytes_opt_(x)

    #define COS_ATTR_out_nonnull_counted_by(x) _Out_writes_(x)
    #define COS_ATTR_out_nullable_counted_by(x) _Out_writes_opt_(x)

    #define COS_ATTR_out_nonnull_sized_by(x) _Out_writes_bytes_(x)
    #define COS_ATTR_out_nullable_sized_by(x) _Out_writes_bytes_opt_(x)

#else

    #define COS_NO_OP_

    #define COS_ATTR_in_nonnull_counted_by(x) COS_NO_OP_
    #define COS_ATTR_in_nullable_counted_by(x) COS_NO_OP_

    #define COS_ATTR_in_nonnull_sized_by(x) COS_NO_OP_
    #define COS_ATTR_in_nullable_sized_by(x) COS_NO_OP_

    #define COS_ATTR_out_nonnull_counted_by(x) COS_NO_OP_
    #define COS_ATTR_out_nullable_counted_by(x) COS_NO_OP_

    #define COS_ATTR_out_nonnull_sized_by(x) COS_NO_OP_
    #define COS_ATTR_out_nullable_sized_by(x) COS_NO_OP_

#endif

#define COS_PARAM_SPEC(direction, nullability, type) \
    COS_PARAM_SPEC_IMPL_(direction, nullability, type, _NONE)

#define COS_PARAM_SPEC_IMPL_(direction, nullability, type, ...) \
    COS_SPEC_GET_NAME_(direction, nullability, type)

#define COS_SPEC_GET_NAME_(direction, nullability, type) \
    COS_SPEC_GET_NAME_IMPL_(direction, nullability, type)

#define COS_SPEC_GET_NAME_IMPL_(direction, nullability, type) \
    COS_ATTR_##direction##_##nullability##_##type

#endif /* LIBCOS_COMMON_COS_API_H */
