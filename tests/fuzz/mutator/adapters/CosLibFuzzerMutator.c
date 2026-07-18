/*
 * Copyright (c) 2026 OpenCOS.
 */

/*
 * The libFuzzer half of the engine-agnostic mutator.
 *
 * libFuzzer takes a custom mutator by linking LLVMFuzzerCustomMutator into the
 * harness. AFL++ instead dlopens a shared object exporting afl_custom_fuzz and
 * friends, which is why the mutation logic lives in an engine-neutral core and
 * each engine gets a thin adapter. This file is the only place in the mutator
 * that may reference a libFuzzer symbol.
 *
 * Note that a missing custom mutator is not a link error -- libFuzzer simply
 * falls back to its own. Confirm it is wired up by looking for
 * "INFO: found LLVMFuzzerCustomMutator" in the startup output.
 */

#include "CosMutator.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef COS_FUZZ_MUTATOR_TARGET
    #define COS_FUZZ_MUTATOR_TARGET CosMutatorTarget_Generic
#endif

/* Provided by libFuzzer. */
size_t
LLVMFuzzerMutate(uint8_t *data, size_t size, size_t max_size);

size_t
LLVMFuzzerCustomMutator(uint8_t *data,
                        size_t size,
                        size_t max_size,
                        unsigned int seed);

/**
 * How often to hand the input to libFuzzer's own mutator instead, in percent.
 *
 * Structure-aware mutation complements byte mutation rather than replacing it:
 * the byte mutator still finds the malformed-input defects that a
 * grammar-respecting mutant never produces.
 */
#define COS_FUZZ_BYTE_MUTATOR_PERCENT 20u

static CosMutator *cos_fuzz_mutator_ = NULL;
static int cos_fuzz_mutator_failed_ = 0;

size_t
LLVMFuzzerCustomMutator(uint8_t *data,
                        size_t size,
                        size_t max_size,
                        unsigned int seed)
{
    if (cos_fuzz_mutator_failed_) {
        return LLVMFuzzerMutate(data, size, max_size);
    }

    if (!cos_fuzz_mutator_) {
        cos_fuzz_mutator_ = cos_mutator_create((CosMutatorTarget)COS_FUZZ_MUTATOR_TARGET,
                                               (uint64_t)seed);
        if (!cos_fuzz_mutator_) {
            cos_fuzz_mutator_failed_ = 1;
            return LLVMFuzzerMutate(data, size, max_size);
        }
    }

    /*
     * Reseed every call, so that the mutant is a pure function of (seed, input)
     * and a run stays reproducible from -seed alone.
     */
    cos_mutator_reseed(cos_fuzz_mutator_, (uint64_t)seed);

    if ((seed % 100u) < COS_FUZZ_BYTE_MUTATOR_PERCENT) {
        return LLVMFuzzerMutate(data, size, max_size);
    }

    size_t out_size = 0;
    const unsigned char * const mutant = cos_mutator_mutate(cos_fuzz_mutator_,
                                                            data,
                                                            size,
                                                            NULL,
                                                            0,
                                                            max_size,
                                                            &out_size);
    if (!mutant || out_size == 0 || out_size > max_size) {
        return LLVMFuzzerMutate(data, size, max_size);
    }

    memcpy(data, mutant, out_size);

    return out_size;
}
