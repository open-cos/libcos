/*
 * Copyright (c) 2026 OpenCOS.
 */

#ifndef LIBCOS_TESTS_FUZZ_COS_MUTATOR_RANDOM_H
#define LIBCOS_TESTS_FUZZ_COS_MUTATOR_RANDOM_H

#include <libcos/common/CosDefines.h>

#include <stdbool.h>
#include <stdint.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * A PCG32 generator.
 *
 * The mutator brings its own generator rather than using rand(): mutation must
 * be a pure function of (seed, input) so that a fuzzing run is reproducible
 * from its seed alone, and a process-global generator would not survive the
 * engine reseeding every call.
 */
typedef struct CosMutRandom {
    uint64_t state;
    uint64_t inc;
} CosMutRandom;

void
cos_mut_random_seed_(CosMutRandom *rng,
                     uint64_t seed);

uint32_t
cos_mut_random_next_(CosMutRandom *rng);

/**
 * Returns a value in [0, bound), or 0 when @p bound is 0.
 */
uint32_t
cos_mut_random_below_(CosMutRandom *rng,
                      uint32_t bound);

/**
 * Returns true with the given probability, in percent. A @p percent of 0 never
 * fires and 100 or more always fires.
 */
bool
cos_mut_random_chance_(CosMutRandom *rng,
                       unsigned int percent);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_TESTS_FUZZ_COS_MUTATOR_RANDOM_H */
