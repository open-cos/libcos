/*
 * Copyright (c) 2026 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_CHECKED_ARITH_H
#define LIBCOS_COMMON_COS_CHECKED_ARITH_H

#include <libcos/common/CosBasicTypes.h>
#include <libcos/common/CosDefines.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @file CosCheckedArith.h
 *
 * Checked integer arithmetic over the two types the parser accumulates:
 * @c CosStreamOffset for byte offsets and @c size_t for in-memory sizes.
 *
 * These follow the C23 @c <stdckdint.h> convention, which is the opposite of
 * the usual one in this library: they return @c true on @b overflow, and
 * @p out is always written -- with the wrapped result when they do overflow.
 * The point is that the wrapped value is computed without ever evaluating the
 * overflowing expression in signed arithmetic, so nothing here is undefined
 * and nothing traps under the Debug build's @c -ftrapv.
 *
 * There is deliberately no @c CosError out-parameter. A generic "overflow"
 * message tells a caller nothing; each site propagates its own error naming
 * the value that was out of range. It also keeps these usable from code with
 * no error channel, such as the tokenizer's literal accumulation loop.
 *
 * Checked arithmetic is the backstop, not the strategy. Bounding a value when
 * it is parsed is cheaper and catches more, because it cuts the input space
 * before any arithmetic happens; see @c cos_xref_validate_subsection_range_.
 */

/**
 * @def COS_HAVE_OVERFLOW_BUILTINS
 *
 * Whether the compiler provides the @c __builtin_*_overflow family.
 *
 * GCC has had these since 5, but only gained @c __has_builtin in 10, so the
 * feature probe alone would silently drop the fast path on GCC 5 through 9.
 * Define @c COS_FORCE_CHECKED_ARITH_FALLBACK to compile the portable path on
 * a compiler that has the builtins; the test suite uses it to exercise both.
 */
#if defined(COS_FORCE_CHECKED_ARITH_FALLBACK)
    #define COS_HAVE_OVERFLOW_BUILTINS 0
#elif COS_HAS_BUILTIN(__builtin_add_overflow) || (defined(__GNUC__) && (__GNUC__ >= 5))
    #define COS_HAVE_OVERFLOW_BUILTINS 1
#else
    #define COS_HAVE_OVERFLOW_BUILTINS 0
#endif

// MARK: - Stream offsets

/**
 * Adds two stream offsets, detecting overflow.
 *
 * @param out Set to the sum, or to the wrapped sum on overflow.
 * @param a The left operand.
 * @param b The right operand.
 *
 * @return @c true if the addition overflowed, @c false otherwise.
 */
COS_STATIC_INLINE bool
cos_ckd_add_off_(CosStreamOffset *out,
                 CosStreamOffset a,
                 CosStreamOffset b)
    COS_ATTR_ACCESS_WRITE_ONLY(1);

/**
 * Subtracts one stream offset from another, detecting overflow.
 *
 * @param out Set to the difference, or to the wrapped difference on overflow.
 * @param a The left operand.
 * @param b The right operand.
 *
 * @return @c true if the subtraction overflowed, @c false otherwise.
 */
COS_STATIC_INLINE bool
cos_ckd_sub_off_(CosStreamOffset *out,
                 CosStreamOffset a,
                 CosStreamOffset b)
    COS_ATTR_ACCESS_WRITE_ONLY(1);

/**
 * Multiplies two stream offsets, detecting overflow.
 *
 * @param out Set to the product, or to the wrapped product on overflow.
 * @param a The left operand.
 * @param b The right operand.
 *
 * @return @c true if the multiplication overflowed, @c false otherwise.
 */
COS_STATIC_INLINE bool
cos_ckd_mul_off_(CosStreamOffset *out,
                 CosStreamOffset a,
                 CosStreamOffset b)
    COS_ATTR_ACCESS_WRITE_ONLY(1);

// MARK: - Sizes

/**
 * Adds two sizes, detecting overflow.
 *
 * @param out Set to the sum, or to the wrapped sum on overflow.
 * @param a The left operand.
 * @param b The right operand.
 *
 * @return @c true if the addition overflowed, @c false otherwise.
 */
COS_STATIC_INLINE bool
cos_ckd_add_size_(size_t *out,
                  size_t a,
                  size_t b)
    COS_ATTR_ACCESS_WRITE_ONLY(1);

/**
 * Subtracts one size from another, detecting overflow.
 *
 * Unsigned subtraction overflows exactly when @p a is less than @p b, so this
 * is the checked spelling of the "compare by subtraction" idiom used
 * throughout the parser.
 *
 * @param out Set to the difference, or to the wrapped difference on overflow.
 * @param a The left operand.
 * @param b The right operand.
 *
 * @return @c true if the subtraction overflowed, @c false otherwise.
 */
COS_STATIC_INLINE bool
cos_ckd_sub_size_(size_t *out,
                  size_t a,
                  size_t b)
    COS_ATTR_ACCESS_WRITE_ONLY(1);

/**
 * Multiplies two sizes, detecting overflow.
 *
 * @param out Set to the product, or to the wrapped product on overflow.
 * @param a The left operand.
 * @param b The right operand.
 *
 * @return @c true if the multiplication overflowed, @c false otherwise.
 */
COS_STATIC_INLINE bool
cos_ckd_mul_size_(size_t *out,
                  size_t a,
                  size_t b)
    COS_ATTR_ACCESS_WRITE_ONLY(1);

// MARK: - Narrowing conversions

/**
 * Converts a stream offset to a size, detecting loss of information.
 *
 * Fails for a negative offset, and for an offset above @c SIZE_MAX where
 * @c size_t is narrower than @c CosStreamOffset -- the 32-bit case, which is
 * why the sign check alone is not enough.
 *
 * @param out Set to the converted value, or to the truncated value on failure.
 * @param value The offset to convert.
 *
 * @return @c true if the value is not representable, @c false otherwise.
 */
COS_STATIC_INLINE bool
cos_ckd_off_to_size_(size_t *out,
                     CosStreamOffset value)
    COS_ATTR_ACCESS_WRITE_ONLY(1);

/**
 * Converts a size to a stream offset, detecting loss of information.
 *
 * @param out Set to the converted value, or to the truncated value on failure.
 * @param value The size to convert.
 *
 * @return @c true if the value is not representable, @c false otherwise.
 */
COS_STATIC_INLINE bool
cos_ckd_size_to_off_(CosStreamOffset *out,
                     size_t value)
    COS_ATTR_ACCESS_WRITE_ONLY(1);

// MARK: - Implementation

#if COS_HAVE_OVERFLOW_BUILTINS

COS_STATIC_INLINE bool
cos_ckd_add_off_(CosStreamOffset *out, CosStreamOffset a, CosStreamOffset b)
{
    return __builtin_add_overflow(a, b, out);
}

COS_STATIC_INLINE bool
cos_ckd_sub_off_(CosStreamOffset *out, CosStreamOffset a, CosStreamOffset b)
{
    return __builtin_sub_overflow(a, b, out);
}

COS_STATIC_INLINE bool
cos_ckd_mul_off_(CosStreamOffset *out, CosStreamOffset a, CosStreamOffset b)
{
    return __builtin_mul_overflow(a, b, out);
}

COS_STATIC_INLINE bool
cos_ckd_add_size_(size_t *out, size_t a, size_t b)
{
    return __builtin_add_overflow(a, b, out);
}

COS_STATIC_INLINE bool
cos_ckd_sub_size_(size_t *out, size_t a, size_t b)
{
    return __builtin_sub_overflow(a, b, out);
}

COS_STATIC_INLINE bool
cos_ckd_mul_size_(size_t *out, size_t a, size_t b)
{
    return __builtin_mul_overflow(a, b, out);
}

COS_STATIC_INLINE bool
cos_ckd_off_to_size_(size_t *out, CosStreamOffset value)
{
    // Adding zero into a differently-typed destination is the builtins' idiom
    // for a checked conversion: the range check is against the result type.
    return __builtin_add_overflow(value, (CosStreamOffset)0, out);
}

COS_STATIC_INLINE bool
cos_ckd_size_to_off_(CosStreamOffset *out, size_t value)
{
    return __builtin_add_overflow(value, (size_t)0, out);
}

#else /* Portable fallback. */

/**
 * Reduces a stream offset pair to the value two's-complement wrapping would
 * produce, without ever evaluating the overflowing signed expression.
 *
 * The arithmetic is done in @c unsigned @c long @c long, where wrapping is
 * defined. Converting the result back is implementation-defined when it
 * exceeds @c COS_STREAM_OFFSET_MAX, but every supported target is two's
 * complement, and C23 makes this the required behaviour.
 */
#define COS_WRAP_OFF_(expr) ((CosStreamOffset)(unsigned long long)(expr))

COS_STATIC_INLINE bool
cos_ckd_add_off_(CosStreamOffset *out, CosStreamOffset a, CosStreamOffset b)
{
    const unsigned long long ua = (unsigned long long)a;
    const unsigned long long ub = (unsigned long long)b;

    *out = COS_WRAP_OFF_(ua + ub);

    // Each direction is tested separately, by subtraction, so that the test
    // itself cannot overflow.
    if (b > 0) {
        return a > COS_STREAM_OFFSET_MAX - b;
    }
    if (b < 0) {
        return a < COS_STREAM_OFFSET_MIN - b;
    }
    return false;
}

COS_STATIC_INLINE bool
cos_ckd_sub_off_(CosStreamOffset *out, CosStreamOffset a, CosStreamOffset b)
{
    const unsigned long long ua = (unsigned long long)a;
    const unsigned long long ub = (unsigned long long)b;

    *out = COS_WRAP_OFF_(ua - ub);

    if (b > 0) {
        return a < COS_STREAM_OFFSET_MIN + b;
    }
    if (b < 0) {
        // Negating COS_STREAM_OFFSET_MIN would itself overflow, so that case
        // is handled before the subtraction is expressed.
        if (b == COS_STREAM_OFFSET_MIN) {
            return a >= 0;
        }
        return a > COS_STREAM_OFFSET_MAX + b;
    }
    return false;
}

COS_STATIC_INLINE bool
cos_ckd_mul_off_(CosStreamOffset *out, CosStreamOffset a, CosStreamOffset b)
{
    const unsigned long long ua = (unsigned long long)a;
    const unsigned long long ub = (unsigned long long)b;

    *out = COS_WRAP_OFF_(ua * ub);

    // Zero is separated out first: it is the one operand the division-based
    // tests below cannot divide by, and it never overflows.
    if (a == 0 || b == 0) {
        return false;
    }

    if (a > 0) {
        if (b > 0) {
            return a > COS_STREAM_OFFSET_MAX / b;
        }
        return b < COS_STREAM_OFFSET_MIN / a;
    }

    if (b > 0) {
        return a < COS_STREAM_OFFSET_MIN / b;
    }
    // Both negative: the product is positive, and a == MIN always overflows
    // because its magnitude has no positive counterpart.
    return a == COS_STREAM_OFFSET_MIN ||
           b == COS_STREAM_OFFSET_MIN ||
           -a > COS_STREAM_OFFSET_MAX / -b;
}

COS_STATIC_INLINE bool
cos_ckd_add_size_(size_t *out, size_t a, size_t b)
{
    *out = a + b;
    return a > SIZE_MAX - b;
}

COS_STATIC_INLINE bool
cos_ckd_sub_size_(size_t *out, size_t a, size_t b)
{
    *out = a - b;
    return a < b;
}

COS_STATIC_INLINE bool
cos_ckd_mul_size_(size_t *out, size_t a, size_t b)
{
    *out = a * b;
    return a != 0 && b > SIZE_MAX / a;
}

COS_STATIC_INLINE bool
cos_ckd_off_to_size_(size_t *out, CosStreamOffset value)
{
    *out = (size_t)value;

    if (value < 0) {
        return true;
    }
    // Widened to a common unsigned type, because which of the two is narrower
    // depends on the target.
    return (unsigned long long)value > (unsigned long long)SIZE_MAX;
}

COS_STATIC_INLINE bool
cos_ckd_size_to_off_(CosStreamOffset *out, size_t value)
{
    *out = (CosStreamOffset)value;

    return (unsigned long long)value > (unsigned long long)COS_STREAM_OFFSET_MAX;
}

#endif /* COS_HAVE_OVERFLOW_BUILTINS */

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_CHECKED_ARITH_H */
