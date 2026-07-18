/*
 * Copyright (c) 2026 OpenCOS.
 */

#include "CosTest.h"

#include "common/CosCheckedArith.h"
#include "common/CosContainerUtils.h"

#include <libcos/common/CosBasicTypes.h>

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

/*
 * These run against whichever backend the build selected. The suite is built
 * a second time with COS_FORCE_CHECKED_ARITH_FALLBACK so the portable path is
 * exercised rather than assumed; see the plan's verification notes.
 */

#define OFF_MAX COS_STREAM_OFFSET_MAX
#define OFF_MIN COS_STREAM_OFFSET_MIN

// MARK: - Offset addition

static int
addOff_withinRange_Succeeds(void)
{
    CosStreamOffset result = 0;

    TEST_EXPECT(!cos_ckd_add_off_(&result, 100, 27));
    TEST_EXPECT(result == 127);

    TEST_EXPECT(!cos_ckd_add_off_(&result, -100, 27));
    TEST_EXPECT(result == -73);

    TEST_EXPECT(!cos_ckd_add_off_(&result, 0, 0));
    TEST_EXPECT(result == 0);

    return EXIT_SUCCESS;
}

static int
addOff_atBoundary_Succeeds(void)
{
    CosStreamOffset result = 0;

    // The largest sum that is still representable, from both directions.
    TEST_EXPECT(!cos_ckd_add_off_(&result, OFF_MAX - 1, 1));
    TEST_EXPECT(result == OFF_MAX);

    TEST_EXPECT(!cos_ckd_add_off_(&result, OFF_MIN + 1, -1));
    TEST_EXPECT(result == OFF_MIN);

    // Adding zero must never be reported as overflow, even at the extremes.
    TEST_EXPECT(!cos_ckd_add_off_(&result, OFF_MAX, 0));
    TEST_EXPECT(result == OFF_MAX);

    TEST_EXPECT(!cos_ckd_add_off_(&result, OFF_MIN, 0));
    TEST_EXPECT(result == OFF_MIN);

    return EXIT_SUCCESS;
}

static int
addOff_pastBoundary_Overflows(void)
{
    CosStreamOffset result = 0;

    TEST_EXPECT(cos_ckd_add_off_(&result, OFF_MAX, 1));
    TEST_EXPECT(cos_ckd_add_off_(&result, 1, OFF_MAX));
    TEST_EXPECT(cos_ckd_add_off_(&result, OFF_MAX, OFF_MAX));

    // Negative overflow toward the minimum.
    TEST_EXPECT(cos_ckd_add_off_(&result, OFF_MIN, -1));
    TEST_EXPECT(cos_ckd_add_off_(&result, -1, OFF_MIN));
    TEST_EXPECT(cos_ckd_add_off_(&result, OFF_MIN, OFF_MIN));

    return EXIT_SUCCESS;
}

// MARK: - Offset subtraction

static int
subOff_withinRange_Succeeds(void)
{
    CosStreamOffset result = 0;

    TEST_EXPECT(!cos_ckd_sub_off_(&result, 127, 27));
    TEST_EXPECT(result == 100);

    TEST_EXPECT(!cos_ckd_sub_off_(&result, 27, 127));
    TEST_EXPECT(result == -100);

    TEST_EXPECT(!cos_ckd_sub_off_(&result, OFF_MAX, OFF_MAX));
    TEST_EXPECT(result == 0);

    return EXIT_SUCCESS;
}

static int
subOff_pastBoundary_Overflows(void)
{
    CosStreamOffset result = 0;

    TEST_EXPECT(cos_ckd_sub_off_(&result, OFF_MIN, 1));
    TEST_EXPECT(cos_ckd_sub_off_(&result, OFF_MAX, -1));

    // Subtracting the minimum is the case whose naive spelling (-b) would
    // itself overflow.
    TEST_EXPECT(cos_ckd_sub_off_(&result, 0, OFF_MIN));
    TEST_EXPECT(cos_ckd_sub_off_(&result, 1, OFF_MIN));

    // ... but it is representable from a sufficiently negative left operand.
    TEST_EXPECT(!cos_ckd_sub_off_(&result, -1, OFF_MIN));
    TEST_EXPECT(result == OFF_MAX);

    return EXIT_SUCCESS;
}

// MARK: - Offset multiplication

static int
mulOff_withinRange_Succeeds(void)
{
    CosStreamOffset result = 0;

    TEST_EXPECT(!cos_ckd_mul_off_(&result, 100, 27));
    TEST_EXPECT(result == 2700);

    TEST_EXPECT(!cos_ckd_mul_off_(&result, -100, 27));
    TEST_EXPECT(result == -2700);

    TEST_EXPECT(!cos_ckd_mul_off_(&result, -100, -27));
    TEST_EXPECT(result == 2700);

    return EXIT_SUCCESS;
}

static int
mulOff_byZero_Succeeds(void)
{
    CosStreamOffset result = 1;

    // Zero is the operand the division-based guard cannot divide by, so it is
    // checked from both sides and against the extremes.
    TEST_EXPECT(!cos_ckd_mul_off_(&result, 0, OFF_MAX));
    TEST_EXPECT(result == 0);

    result = 1;
    TEST_EXPECT(!cos_ckd_mul_off_(&result, OFF_MAX, 0));
    TEST_EXPECT(result == 0);

    result = 1;
    TEST_EXPECT(!cos_ckd_mul_off_(&result, 0, OFF_MIN));
    TEST_EXPECT(result == 0);

    result = 1;
    TEST_EXPECT(!cos_ckd_mul_off_(&result, OFF_MIN, 0));
    TEST_EXPECT(result == 0);

    return EXIT_SUCCESS;
}

static int
mulOff_pastBoundary_Overflows(void)
{
    CosStreamOffset result = 0;

    TEST_EXPECT(cos_ckd_mul_off_(&result, OFF_MAX, 2));
    TEST_EXPECT(cos_ckd_mul_off_(&result, 2, OFF_MAX));
    TEST_EXPECT(cos_ckd_mul_off_(&result, OFF_MAX, -2));
    TEST_EXPECT(cos_ckd_mul_off_(&result, -2, OFF_MAX));

    // The minimum has no positive counterpart, so negating it by multiply
    // overflows even though the operands look small.
    TEST_EXPECT(cos_ckd_mul_off_(&result, OFF_MIN, -1));
    TEST_EXPECT(cos_ckd_mul_off_(&result, -1, OFF_MIN));
    TEST_EXPECT(cos_ckd_mul_off_(&result, OFF_MIN, 2));

    // Multiplying the minimum by 1 is fine, and must not be caught by the
    // both-negative guard.
    TEST_EXPECT(!cos_ckd_mul_off_(&result, OFF_MIN, 1));
    TEST_EXPECT(result == OFF_MIN);

    return EXIT_SUCCESS;
}

// MARK: - Size arithmetic

static int
addSize_boundary_Detected(void)
{
    size_t result = 0;

    TEST_EXPECT(!cos_ckd_add_size_(&result, 100, 27));
    TEST_EXPECT(result == 127);

    TEST_EXPECT(!cos_ckd_add_size_(&result, SIZE_MAX - 1, 1));
    TEST_EXPECT(result == SIZE_MAX);

    TEST_EXPECT(!cos_ckd_add_size_(&result, SIZE_MAX, 0));
    TEST_EXPECT(result == SIZE_MAX);

    TEST_EXPECT(cos_ckd_add_size_(&result, SIZE_MAX, 1));
    TEST_EXPECT(cos_ckd_add_size_(&result, SIZE_MAX, SIZE_MAX));

    return EXIT_SUCCESS;
}

static int
subSize_boundary_Detected(void)
{
    size_t result = 0;

    TEST_EXPECT(!cos_ckd_sub_size_(&result, 127, 27));
    TEST_EXPECT(result == 100);

    TEST_EXPECT(!cos_ckd_sub_size_(&result, 0, 0));
    TEST_EXPECT(result == 0);

    // Unsigned subtraction overflows exactly when a < b.
    TEST_EXPECT(cos_ckd_sub_size_(&result, 0, 1));
    TEST_EXPECT(cos_ckd_sub_size_(&result, 27, 127));

    return EXIT_SUCCESS;
}

static int
mulSize_boundary_Detected(void)
{
    size_t result = 0;

    TEST_EXPECT(!cos_ckd_mul_size_(&result, 100, 27));
    TEST_EXPECT(result == 2700);

    // a == 0 must short-circuit before the division guard.
    result = 1;
    TEST_EXPECT(!cos_ckd_mul_size_(&result, 0, SIZE_MAX));
    TEST_EXPECT(result == 0);

    result = 1;
    TEST_EXPECT(!cos_ckd_mul_size_(&result, SIZE_MAX, 0));
    TEST_EXPECT(result == 0);

    TEST_EXPECT(!cos_ckd_mul_size_(&result, SIZE_MAX, 1));
    TEST_EXPECT(result == SIZE_MAX);

    TEST_EXPECT(cos_ckd_mul_size_(&result, SIZE_MAX, 2));
    TEST_EXPECT(cos_ckd_mul_size_(&result, 2, SIZE_MAX));
    TEST_EXPECT(cos_ckd_mul_size_(&result, SIZE_MAX, SIZE_MAX));

    return EXIT_SUCCESS;
}

// MARK: - Narrowing conversions

static int
offToSize_negative_Fails(void)
{
    size_t result = 0;

    TEST_EXPECT(cos_ckd_off_to_size_(&result, -1));
    TEST_EXPECT(cos_ckd_off_to_size_(&result, OFF_MIN));

    TEST_EXPECT(!cos_ckd_off_to_size_(&result, 0));
    TEST_EXPECT(result == 0);

    TEST_EXPECT(!cos_ckd_off_to_size_(&result, 127));
    TEST_EXPECT(result == 127);

    return EXIT_SUCCESS;
}

static int
offToSize_aboveSizeMax_Fails(void)
{
    size_t result = 0;

    /*
     * Whether OFF_MAX is representable as a size_t is target-dependent: it is
     * on LP64, and is not where size_t is 32 bits. Assert the behaviour that
     * actually applies rather than assuming a word size.
     */
    if ((unsigned long long)SIZE_MAX >= (unsigned long long)OFF_MAX) {
        TEST_EXPECT(!cos_ckd_off_to_size_(&result, OFF_MAX));
        TEST_EXPECT(result == (size_t)OFF_MAX);
    }
    else {
        TEST_EXPECT(cos_ckd_off_to_size_(&result, OFF_MAX));

        // The first value that does not fit.
        const CosStreamOffset just_past = (CosStreamOffset)SIZE_MAX + 1;
        TEST_EXPECT(cos_ckd_off_to_size_(&result, just_past));

        TEST_EXPECT(!cos_ckd_off_to_size_(&result, (CosStreamOffset)SIZE_MAX));
        TEST_EXPECT(result == SIZE_MAX);
    }

    return EXIT_SUCCESS;
}

static int
sizeToOff_boundary_Detected(void)
{
    CosStreamOffset result = 0;

    TEST_EXPECT(!cos_ckd_size_to_off_(&result, 0));
    TEST_EXPECT(result == 0);

    TEST_EXPECT(!cos_ckd_size_to_off_(&result, 127));
    TEST_EXPECT(result == 127);

    if ((unsigned long long)SIZE_MAX > (unsigned long long)OFF_MAX) {
        // A 128-bit-free target where size_t is wider: SIZE_MAX does not fit.
        TEST_EXPECT(cos_ckd_size_to_off_(&result, SIZE_MAX));
    }
    else {
        TEST_EXPECT(!cos_ckd_size_to_off_(&result, SIZE_MAX));
        TEST_EXPECT(result == (CosStreamOffset)SIZE_MAX);
    }

    return EXIT_SUCCESS;
}

// MARK: - Container capacity rounding

/*
 * Not part of CosCheckedArith.h, but the same failure mode: rounding a
 * capacity up used to compute 1 << fls(x), which is undefined once the shift
 * reaches the width of the type and returned 1. A container that rounded a
 * large request down to 1 reported the resize as successful and was then
 * written past the end of its buffer.
 */
static int
roundCapacity_neverReturnsLessThanRequested(void)
{
    const size_t highest_pow2 = ((size_t)1) << ((sizeof(size_t) * CHAR_BIT) - 1);

    const size_t cases[] = {
        0,
        4,
        100,
        highest_pow2 / 2,
        highest_pow2 - 2,
        highest_pow2 - 1,
        highest_pow2,
        SIZE_MAX - 1,
        SIZE_MAX,
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        const size_t rounded = cos_container_round_capacity_(cases[i]);

        // The invariant every caller depends on.
        TEST_EXPECT(rounded >= cases[i]);
        TEST_EXPECT(rounded >= 4);
    }

    // Ordinary values still round up to a power of two.
    TEST_EXPECT(cos_container_round_capacity_(100) == 128);
    TEST_EXPECT(cos_container_round_capacity_(128) == 256);

    // Beyond the largest representable power of two, it saturates rather than
    // wrapping, so the caller's allocation is what fails.
    TEST_EXPECT(cos_container_round_capacity_(SIZE_MAX) == SIZE_MAX);
    TEST_EXPECT(cos_container_round_capacity_(highest_pow2) == SIZE_MAX);

    return EXIT_SUCCESS;
}

TEST_MAIN()
{
    /* Offset addition */
    TEST_EXPECT(addOff_withinRange_Succeeds() == EXIT_SUCCESS);
    TEST_EXPECT(addOff_atBoundary_Succeeds() == EXIT_SUCCESS);
    TEST_EXPECT(addOff_pastBoundary_Overflows() == EXIT_SUCCESS);

    /* Offset subtraction */
    TEST_EXPECT(subOff_withinRange_Succeeds() == EXIT_SUCCESS);
    TEST_EXPECT(subOff_pastBoundary_Overflows() == EXIT_SUCCESS);

    /* Offset multiplication */
    TEST_EXPECT(mulOff_withinRange_Succeeds() == EXIT_SUCCESS);
    TEST_EXPECT(mulOff_byZero_Succeeds() == EXIT_SUCCESS);
    TEST_EXPECT(mulOff_pastBoundary_Overflows() == EXIT_SUCCESS);

    /* Size arithmetic */
    TEST_EXPECT(addSize_boundary_Detected() == EXIT_SUCCESS);
    TEST_EXPECT(subSize_boundary_Detected() == EXIT_SUCCESS);
    TEST_EXPECT(mulSize_boundary_Detected() == EXIT_SUCCESS);

    /* Narrowing conversions */
    TEST_EXPECT(offToSize_negative_Fails() == EXIT_SUCCESS);
    TEST_EXPECT(offToSize_aboveSizeMax_Fails() == EXIT_SUCCESS);
    TEST_EXPECT(sizeToOff_boundary_Detected() == EXIT_SUCCESS);

    /* Container capacity rounding */
    TEST_EXPECT(roundCapacity_neverReturnsLessThanRequested() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END
