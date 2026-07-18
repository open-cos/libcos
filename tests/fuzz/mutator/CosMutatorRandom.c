/*
 * Copyright (c) 2026 OpenCOS.
 */

#include "CosMutatorRandom.h"

#include "common/Assert.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

/* The stream constant from the PCG reference implementation. */
#define COS_MUT_RANDOM_INC UINT64_C(1442695040888963407)

#define COS_MUT_RANDOM_MULT UINT64_C(6364136223846793005)

void
cos_mut_random_seed_(CosMutRandom *rng,
                     uint64_t seed)
{
    COS_IMPL_PARAM_CHECK(rng != NULL);

    rng->state = 0;
    rng->inc = COS_MUT_RANDOM_INC;

    (void)cos_mut_random_next_(rng);
    rng->state += seed;
    (void)cos_mut_random_next_(rng);
}

uint32_t
cos_mut_random_next_(CosMutRandom *rng)
{
    COS_IMPL_PARAM_CHECK(rng != NULL);

    const uint64_t previous = rng->state;

    rng->state = (previous * COS_MUT_RANDOM_MULT) + (rng->inc | UINT64_C(1));

    const uint32_t xorshifted = (uint32_t)(((previous >> 18u) ^ previous) >> 27u);
    const uint32_t rot = (uint32_t)(previous >> 59u);

    return (xorshifted >> rot) | (xorshifted << ((0u - rot) & 31u));
}

uint32_t
cos_mut_random_below_(CosMutRandom *rng,
                      uint32_t bound)
{
    COS_IMPL_PARAM_CHECK(rng != NULL);

    if (bound == 0) {
        return 0;
    }

    /*
     * Rejection sampling, so that the result is uniform rather than biased
     * toward the low end as a plain modulo would be.
     */
    const uint32_t threshold = (0u - bound) % bound;

    for (;;) {
        const uint32_t value = cos_mut_random_next_(rng);
        if (value >= threshold) {
            return value % bound;
        }
    }
}

bool
cos_mut_random_chance_(CosMutRandom *rng,
                       unsigned int percent)
{
    COS_IMPL_PARAM_CHECK(rng != NULL);

    if (percent == 0) {
        return false;
    }
    if (percent >= 100) {
        return true;
    }

    return cos_mut_random_below_(rng, 100) < (uint32_t)percent;
}

COS_ASSUME_NONNULL_END
