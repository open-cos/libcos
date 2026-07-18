/*
 * Copyright (c) 2026 OpenCOS.
 */

#ifndef LIBCOS_TESTS_FUZZ_COS_MUTATOR_H
#define LIBCOS_TESTS_FUZZ_COS_MUTATOR_H

#include <libcos/common/CosDefines.h>

#include <stddef.h>
#include <stdint.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * Selects which grammar surface a mutator instance targets.
 *
 * The file-level repair pass applies only to @c CosMutatorTarget_Parser, whose
 * corpus is whole PDF files. The other targets are fed fragments, which a
 * file-level repair would corrupt rather than fix.
 */
typedef enum CosMutatorTarget {
    CosMutatorTarget_Generic = 0,
    CosMutatorTarget_Tokenizer,
    CosMutatorTarget_ObjParser,
    CosMutatorTarget_Parser,
} CosMutatorTarget;

typedef struct CosMutator CosMutator;

CosMutator * COS_Nullable
cos_mutator_create(CosMutatorTarget target,
                   uint64_t seed);

void
cos_mutator_destroy(CosMutator * COS_Nullable mutator);

/**
 * Reseeds the generator.
 *
 * The engines hand a fresh seed to every call. Reseeding on each one makes the
 * output a pure function of (seed, input), which is what keeps a run
 * reproducible from its seed alone.
 */
void
cos_mutator_reseed(CosMutator *mutator,
                   uint64_t seed);

/**
 * Sets the probability, in percent, that the self-consistency repair pass runs
 * after a mutation. Values above 100 are clamped. Defaults to 75.
 *
 * Repair is deliberately not certain: a fully repaired corpus would stop
 * exercising the bad-offset and missing-marker paths that the parser rejects in
 * its first two phases.
 */
void
cos_mutator_set_repair_percent(CosMutator *mutator,
                               unsigned int percent);

/**
 * Produces one mutant of @p data.
 *
 * @p add_data, when non-NULL, is a second corpus entry the splice operators may
 * draw from. It is unused in this version, and is present so that the AFL++
 * adapter -- whose @c afl_custom_fuzz receives an @c add_buf -- needs no change
 * to the core.
 *
 * @return A pointer owned by @p mutator and valid until the next call on it, or
 * @c NULL when no mutation could be produced, in which case the caller should
 * fall back to its own mutator. The length written to @p out_size never exceeds
 * @p max_size.
 */
const unsigned char * COS_Nullable
cos_mutator_mutate(CosMutator *mutator,
                   const unsigned char *data,
                   size_t size,
                   const unsigned char * COS_Nullable add_data,
                   size_t add_size,
                   size_t max_size,
                   size_t *out_size)
    COS_ATTR_ACCESS_WRITE_ONLY(7);

/**
 * Names the operator applied by the most recent call, for AFL++'s
 * @c afl_custom_describe and for test diagnostics. Never @c NULL.
 */
const char *
cos_mutator_get_last_operator(const CosMutator *mutator);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_TESTS_FUZZ_COS_MUTATOR_H */
