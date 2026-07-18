/*
 * Copyright (c) 2026 OpenCOS.
 */

#ifndef LIBCOS_TESTS_FUZZ_COS_MUTATOR_PRIVATE_H
#define LIBCOS_TESTS_FUZZ_COS_MUTATOR_PRIVATE_H

#include "CosMutator.h"
#include "CosMutatorBuffer.h"
#include "CosMutatorLex.h"
#include "CosMutatorRandom.h"

#include <libcos/common/CosDefines.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosMutator {
    CosMutatorTarget target;
    CosMutRandom rng;
    unsigned int repair_percent;
    const char *last_operator;

    /* The mutant under construction, and the pre-repair snapshot. */
    CosMutBuffer buffer;
    CosMutBuffer snapshot;

    /*
     * Staging for operators that copy one region of the working buffer over
     * another; cos_mut_buffer_splice_ forbids an aliased source. Two of them,
     * because a swap must capture both spans before it edits either.
     */
    CosMutBuffer scratch;
    CosMutBuffer scratch_alt;

    /* Reused across calls, so that steady-state mutation does not allocate. */
    CosMutSpan * COS_Nullable spans;
    size_t span_capacity;
    size_t span_count;

    CosMutFileView view;
};

/**
 * Re-scans the working buffer into the mutator's span arena and rebuilds the
 * file view.
 *
 * Operators invalidate the spans they were dispatched with, so this runs before
 * each one rather than once per call.
 *
 * @return false when the buffer is empty or the arena could not be grown.
 */
bool
cos_mutator_relex_(CosMutator *mutator);

/**
 * Picks a span at random whose type is in @p type_mask, expressed as a bitmask
 * of @c (1u << CosMutSpanType).
 *
 * @return false when no span matches.
 */
bool
cos_mutator_pick_span_(CosMutator *mutator,
                       uint32_t type_mask,
                       size_t *out_index)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

/** The set of spans that carry content, as opposed to layout. */
#define COS_MUT_TYPE_MASK_VALUE                     \
    ((1u << CosMutSpanType_Integer) |               \
     (1u << CosMutSpanType_Real) |                  \
     (1u << CosMutSpanType_Name) |                  \
     (1u << CosMutSpanType_LiteralString) |         \
     (1u << CosMutSpanType_HexString) |             \
     (1u << CosMutSpanType_Keyword))

#define COS_MUT_TYPE_MASK_NUMBER      \
    ((1u << CosMutSpanType_Integer) | \
     (1u << CosMutSpanType_Real))

/*
 * The operators. Each returns false when it does not apply to the current
 * buffer -- no numbers to perturb, no keywords to retype -- which the dispatch
 * loop treats as a signal to try another one.
 */

bool
cos_mut_op_span_swap_(CosMutator *mutator);

bool
cos_mut_op_span_duplicate_(CosMutator *mutator);

bool
cos_mut_op_span_delete_(CosMutator *mutator);

bool
cos_mut_op_number_boundary_(CosMutator *mutator);

bool
cos_mut_op_keyword_retype_(CosMutator *mutator);

bool
cos_mut_op_container_nest_(CosMutator *mutator);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_TESTS_FUZZ_COS_MUTATOR_PRIVATE_H */
