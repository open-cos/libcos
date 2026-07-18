/*
 * Copyright (c) 2026 OpenCOS.
 */

#include "CosMutator-Private.h"

#include "CosMutatorRepair.h"
#include "common/Assert.h"

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

#define COS_MUT_TARGET_BIT(target) (1u << (unsigned int)(target))

#define COS_MUT_ALL_TARGETS                        \
    (COS_MUT_TARGET_BIT(CosMutatorTarget_Generic) | \
     COS_MUT_TARGET_BIT(CosMutatorTarget_Tokenizer) | \
     COS_MUT_TARGET_BIT(CosMutatorTarget_ObjParser) | \
     COS_MUT_TARGET_BIT(CosMutatorTarget_Parser))

typedef bool (*CosMutOpFunc)(CosMutator *mutator);

typedef struct CosMutOp {
    const char *name;
    CosMutOpFunc func;
    unsigned int weight;
    unsigned int targets;
} CosMutOp;

/*
 * Every operator in this version works on body syntax alone, so all four
 * targets share the table. The file-level operators deferred to a later version
 * are the ones that will need the target mask to start doing work.
 */
static const CosMutOp cos_mut_ops_[] = {
    {"span_swap", cos_mut_op_span_swap_, 20, COS_MUT_ALL_TARGETS},
    {"span_duplicate", cos_mut_op_span_duplicate_, 20, COS_MUT_ALL_TARGETS},
    {"span_delete", cos_mut_op_span_delete_, 20, COS_MUT_ALL_TARGETS},
    {"number_boundary", cos_mut_op_number_boundary_, 20, COS_MUT_ALL_TARGETS},
    {"keyword_retype", cos_mut_op_keyword_retype_, 15, COS_MUT_ALL_TARGETS},
    {"container_nest", cos_mut_op_container_nest_, 5, COS_MUT_ALL_TARGETS},
};

#define COS_MUT_OP_COUNT (sizeof(cos_mut_ops_) / sizeof(cos_mut_ops_[0]))

/** The number of stacked operators per call is drawn from this bound. */
#define COS_MUT_MAX_STACKED_OPS 4

/** How many times dispatch retries after an operator declines to apply. */
#define COS_MUT_MAX_OP_RETRIES 4

CosMutator *
cos_mutator_create(CosMutatorTarget target,
                   uint64_t seed)
{
    CosMutator * const mutator = calloc(1, sizeof(CosMutator));
    if (!mutator) {
        return NULL;
    }

    mutator->target = target;
    mutator->repair_percent = 75;
    mutator->last_operator = "none";

    cos_mut_random_seed_(&mutator->rng, seed);

    cos_mut_buffer_init_(&mutator->buffer);
    cos_mut_buffer_init_(&mutator->snapshot);
    cos_mut_buffer_init_(&mutator->scratch);
    cos_mut_buffer_init_(&mutator->scratch_alt);

    mutator->spans = calloc(COS_MUT_MAX_SPANS, sizeof(CosMutSpan));
    if (!mutator->spans) {
        free(mutator);
        return NULL;
    }
    mutator->span_capacity = COS_MUT_MAX_SPANS;

    return mutator;
}

void
cos_mutator_destroy(CosMutator * COS_Nullable mutator)
{
    if (!mutator) {
        return;
    }

    cos_mut_buffer_free_(&mutator->buffer);
    cos_mut_buffer_free_(&mutator->snapshot);
    cos_mut_buffer_free_(&mutator->scratch);
    cos_mut_buffer_free_(&mutator->scratch_alt);

    free(mutator->spans);
    free(mutator);
}

void
cos_mutator_reseed(CosMutator *mutator,
                   uint64_t seed)
{
    COS_API_PARAM_CHECK(mutator != NULL);
    if (COS_UNLIKELY(!mutator)) {
        return;
    }

    cos_mut_random_seed_(&mutator->rng, seed);
}

void
cos_mutator_set_repair_percent(CosMutator *mutator,
                               unsigned int percent)
{
    COS_API_PARAM_CHECK(mutator != NULL);
    if (COS_UNLIKELY(!mutator)) {
        return;
    }

    mutator->repair_percent = (percent > 100) ? 100 : percent;
}

const char *
cos_mutator_get_last_operator(const CosMutator *mutator)
{
    COS_API_PARAM_CHECK(mutator != NULL);
    if (COS_UNLIKELY(!mutator)) {
        return "none";
    }

    return mutator->last_operator;
}

bool
cos_mutator_relex_(CosMutator *mutator)
{
    COS_IMPL_PARAM_CHECK(mutator != NULL);

    /*
     * Bind both to locals: the checks below do not narrow the nullability of a
     * struct member as far as the compiler is concerned.
     */
    CosMutSpan * const spans = mutator->spans;
    const unsigned char * const data = mutator->buffer.data;

    if (!spans || !data || mutator->buffer.length == 0) {
        mutator->span_count = 0;
        return false;
    }

    mutator->span_count = cos_mut_lex_scan_(data,
                                            mutator->buffer.length,
                                            spans,
                                            mutator->span_capacity);

    if (mutator->span_count == 0) {
        return false;
    }

    cos_mut_view_build_(&mutator->view,
                        data,
                        mutator->buffer.length,
                        spans,
                        mutator->span_count);

    return true;
}

bool
cos_mutator_pick_span_(CosMutator *mutator,
                       uint32_t type_mask,
                       size_t *out_index)
{
    COS_IMPL_PARAM_CHECK(mutator != NULL);
    COS_IMPL_PARAM_CHECK(out_index != NULL);

    if (!mutator->spans) {
        return false;
    }

    size_t match_count = 0;
    for (size_t i = 0; i < mutator->span_count; i++) {
        if ((type_mask & (1u << mutator->spans[i].type)) != 0) {
            match_count++;
        }
    }

    if (match_count == 0) {
        return false;
    }

    size_t wanted = (size_t)cos_mut_random_below_(&mutator->rng,
                                                  (uint32_t)match_count);

    for (size_t i = 0; i < mutator->span_count; i++) {
        if ((type_mask & (1u << mutator->spans[i].type)) == 0) {
            continue;
        }
        if (wanted == 0) {
            *out_index = i;
            return true;
        }
        wanted--;
    }

    return false;
}

/**
 * A self-contained byte-level mutation, for inputs with no COS structure worth
 * preserving.
 *
 * This deliberately does not call libFuzzer's LLVMFuzzerMutate. The core has to
 * stay free of engine symbols so that the AFL++ adapter -- a shared object
 * dlopened by afl-fuzz, where that symbol does not exist -- can use it
 * unchanged.
 */
static bool
cos_mutator_fallback_(CosMutator *mutator)
{
    COS_IMPL_PARAM_CHECK(mutator != NULL);

    static const unsigned char interesting[] = {
        0x00, 0x01, 0x7F, 0x80, 0xFF,
        ' ', '\n', '%', '<', '>'};

    CosMutBuffer * const buffer = &mutator->buffer;

    if (buffer->length == 0) {
        /* Nothing to perturb: seed a byte so later calls have something. */
        static const unsigned char seed_byte = '%';
        return cos_mut_buffer_splice_(buffer, 0, 0, &seed_byte, 1);
    }

    const uint32_t choice = cos_mut_random_below_(&mutator->rng, 5);
    const size_t at = (size_t)cos_mut_random_below_(&mutator->rng,
                                                    (uint32_t)buffer->length);

    switch (choice) {
        case 0: {
            /* Bit flip. */
            const unsigned int bit = cos_mut_random_below_(&mutator->rng, 8);
            buffer->data[at] ^= (unsigned char)(1u << bit);
            return true;
        }
        case 1: {
            /* Interesting byte. */
            const size_t pick = (size_t)cos_mut_random_below_(
                &mutator->rng,
                (uint32_t)(sizeof(interesting) / sizeof(interesting[0])));
            buffer->data[at] = interesting[pick];
            return true;
        }
        case 2: {
            /* Insert a short run of one byte. */
            unsigned char run[16];
            const size_t run_length =
                1u + (size_t)cos_mut_random_below_(&mutator->rng, 15);
            memset(run, buffer->data[at], run_length);
            return cos_mut_buffer_splice_(buffer, at, 0, run, run_length);
        }
        case 3: {
            /* Delete a range. */
            const size_t remaining = buffer->length - at;
            const size_t remove =
                1u + (size_t)cos_mut_random_below_(
                         &mutator->rng,
                         (uint32_t)((remaining < 16) ? remaining : 16));
            return cos_mut_buffer_splice_(buffer, at, remove, NULL, 0);
        }
        default: {
            /* Byte-wise arithmetic. */
            const int delta =
                1 + (int)cos_mut_random_below_(&mutator->rng, 35);
            buffer->data[at] = (unsigned char)(buffer->data[at] +
                                               (cos_mut_random_chance_(&mutator->rng, 50)
                                                    ? delta
                                                    : -delta));
            return true;
        }
    }
}

/**
 * Picks one operator by weight, restricted to those the target enables.
 */
static const CosMutOp * COS_Nullable
cos_mutator_pick_op_(CosMutator *mutator)
{
    COS_IMPL_PARAM_CHECK(mutator != NULL);

    const unsigned int target_bit = COS_MUT_TARGET_BIT(mutator->target);

    unsigned int total = 0;
    for (size_t i = 0; i < COS_MUT_OP_COUNT; i++) {
        if ((cos_mut_ops_[i].targets & target_bit) != 0) {
            total += cos_mut_ops_[i].weight;
        }
    }

    if (total == 0) {
        return NULL;
    }

    uint32_t roll = cos_mut_random_below_(&mutator->rng, total);

    for (size_t i = 0; i < COS_MUT_OP_COUNT; i++) {
        if ((cos_mut_ops_[i].targets & target_bit) == 0) {
            continue;
        }
        if (roll < cos_mut_ops_[i].weight) {
            return &cos_mut_ops_[i];
        }
        roll -= cos_mut_ops_[i].weight;
    }

    return NULL;
}

/**
 * Applies one operator, retrying when the chosen one does not apply.
 */
static bool
cos_mutator_apply_op_(CosMutator *mutator)
{
    COS_IMPL_PARAM_CHECK(mutator != NULL);

    for (unsigned int attempt = 0; attempt < COS_MUT_MAX_OP_RETRIES; attempt++) {
        /* Operators invalidate the spans, so re-scan before each attempt. */
        if (!cos_mutator_relex_(mutator)) {
            return false;
        }

        const CosMutOp * const op = cos_mutator_pick_op_(mutator);
        if (!op) {
            return false;
        }

        if (op->func(mutator)) {
            mutator->last_operator = op->name;
            return true;
        }
    }

    return false;
}

const unsigned char *
cos_mutator_mutate(CosMutator *mutator,
                   const unsigned char *data,
                   size_t size,
                   const unsigned char * COS_Nullable add_data,
                   size_t add_size,
                   size_t max_size,
                   size_t *out_size)
{
    COS_API_PARAM_CHECK(mutator != NULL);
    COS_API_PARAM_CHECK(data != NULL || size == 0);
    COS_API_PARAM_CHECK(out_size != NULL);

    if (COS_UNLIKELY(!mutator || !out_size || (!data && size > 0))) {
        return NULL;
    }

    /* Reserved for the splice operators a later version will add. */
    (void)add_data;
    (void)add_size;

    *out_size = 0;
    mutator->last_operator = "none";

    if (max_size == 0) {
        return NULL;
    }

    cos_mut_buffer_set_max_(&mutator->buffer, max_size);
    if (!cos_mut_buffer_assign_(&mutator->buffer, data, size)) {
        return NULL;
    }

    /*
     * Stack a few operators. One edit rarely moves an input far enough to reach
     * new code, and the count is drawn so that the cheap single-edit case stays
     * the common one.
     */
    const uint32_t op_count =
        1u + cos_mut_random_below_(&mutator->rng, COS_MUT_MAX_STACKED_OPS);

    bool applied = false;
    for (uint32_t i = 0; i < op_count; i++) {
        if (cos_mutator_apply_op_(mutator)) {
            applied = true;
        }
    }

    if (!applied) {
        /* No COS structure to work with; fall back to byte mutation. */
        if (!cos_mutator_fallback_(mutator)) {
            return NULL;
        }
        mutator->last_operator = "byte_fallback";
    }

    /*
     * Repair only whole-file inputs. The other targets are fed fragments, which
     * a file-level repair would corrupt rather than fix.
     */
    if (mutator->target == CosMutatorTarget_Parser &&
        cos_mut_random_chance_(&mutator->rng, mutator->repair_percent)) {
        CosMutRepairFlags flags = CosMutRepairFlag_None;
        for (unsigned int bit = 0; bit < COS_MUT_REPAIR_STAGE_COUNT; bit++) {
            if (cos_mut_random_chance_(&mutator->rng, 85)) {
                flags |= (CosMutRepairFlags)(1u << bit);
            }
        }
        (void)cos_mut_repair_apply_(mutator, flags);
    }

    if (mutator->buffer.length == 0) {
        return NULL;
    }

    *out_size = mutator->buffer.length;

    return mutator->buffer.data;
}

COS_ASSUME_NONNULL_END
