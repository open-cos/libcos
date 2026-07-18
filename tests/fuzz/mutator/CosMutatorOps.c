/*
 * Copyright (c) 2026 OpenCOS.
 */

#include "CosMutator-Private.h"

#include "common/Assert.h"

#include <stdio.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

/**
 * Copies a region of the working buffer into the scratch buffer.
 *
 * Operators that move bytes within the buffer must stage them here first:
 * cos_mut_buffer_splice_ may reallocate or shift the storage its source points
 * into.
 */
static bool
cos_mut_op_stage_into_(CosMutator *mutator,
                       CosMutBuffer *staging,
                       size_t offset,
                       size_t length)
{
    COS_IMPL_PARAM_CHECK(mutator != NULL);
    COS_IMPL_PARAM_CHECK(staging != NULL);

    if (offset > mutator->buffer.length ||
        length > (mutator->buffer.length - offset)) {
        return false;
    }

    cos_mut_buffer_set_max_(staging, mutator->buffer.max_length);

    return cos_mut_buffer_assign_(staging,
                                  mutator->buffer.data + offset,
                                  length);
}

static bool
cos_mut_op_stage_(CosMutator *mutator,
                  size_t offset,
                  size_t length)
{
    COS_IMPL_PARAM_CHECK(mutator != NULL);

    return cos_mut_op_stage_into_(mutator, &mutator->scratch, offset, length);
}

/**
 * Picks a second span of the same type as @p first, if there is one.
 */
static bool
cos_mut_op_pick_sibling_(CosMutator *mutator,
                         size_t first,
                         size_t *out_index)
{
    COS_IMPL_PARAM_CHECK(mutator != NULL);
    COS_IMPL_PARAM_CHECK(out_index != NULL);

    const CosMutSpan * const spans = mutator->spans;
    if (!spans || first >= mutator->span_count) {
        return false;
    }

    const uint16_t type = spans[first].type;

    size_t match_count = 0;
    for (size_t i = 0; i < mutator->span_count; i++) {
        if (i != first && spans[i].type == type) {
            match_count++;
        }
    }

    if (match_count == 0) {
        return false;
    }

    size_t wanted = (size_t)cos_mut_random_below_(&mutator->rng,
                                                  (uint32_t)match_count);

    for (size_t i = 0; i < mutator->span_count; i++) {
        if (i == first || spans[i].type != type) {
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

bool
cos_mut_op_span_swap_(CosMutator *mutator)
{
    COS_IMPL_PARAM_CHECK(mutator != NULL);

    size_t first = 0;
    if (!cos_mutator_pick_span_(mutator, COS_MUT_TYPE_MASK_VALUE, &first)) {
        return false;
    }

    size_t second = 0;
    if (!cos_mut_op_pick_sibling_(mutator, first, &second)) {
        return false;
    }

    if (first > second) {
        const size_t swap = first;
        first = second;
        second = swap;
    }

    const CosMutSpan a = mutator->spans[first];
    const CosMutSpan b = mutator->spans[second];

    /*
     * Stage both spans before editing either: the first splice overwrites the
     * bytes the second one needs to read.
     */
    if (!cos_mut_op_stage_into_(mutator, &mutator->scratch,
                                a.offset, a.length) ||
        !cos_mut_op_stage_into_(mutator, &mutator->scratch_alt,
                                b.offset, b.length)) {
        return false;
    }

    /*
     * Replace the later span first: rewriting the earlier one would shift the
     * later one's offset when the two differ in length.
     */
    if (!cos_mut_buffer_splice_(&mutator->buffer,
                                b.offset,
                                b.length,
                                mutator->scratch.data,
                                mutator->scratch.length)) {
        return false;
    }

    return cos_mut_buffer_splice_(&mutator->buffer,
                                  a.offset,
                                  a.length,
                                  mutator->scratch_alt.data,
                                  mutator->scratch_alt.length);
}

bool
cos_mut_op_span_duplicate_(CosMutator *mutator)
{
    COS_IMPL_PARAM_CHECK(mutator != NULL);

    size_t source = 0;
    if (!cos_mutator_pick_span_(mutator, COS_MUT_TYPE_MASK_VALUE, &source)) {
        return false;
    }

    const CosMutSpan span = mutator->spans[source];

    if (!cos_mut_op_stage_(mutator, span.offset, span.length)) {
        return false;
    }

    /* Insert at a span boundary, so that the copy does not split a token. */
    const size_t target = (size_t)cos_mut_random_below_(
        &mutator->rng,
        (uint32_t)mutator->span_count);
    const size_t at = mutator->spans[target].offset;

    return cos_mut_buffer_splice_(&mutator->buffer,
                                  at,
                                  0,
                                  mutator->scratch.data,
                                  mutator->scratch.length);
}

bool
cos_mut_op_span_delete_(CosMutator *mutator)
{
    COS_IMPL_PARAM_CHECK(mutator != NULL);

    size_t index = 0;
    if (!cos_mutator_pick_span_(mutator, COS_MUT_TYPE_MASK_VALUE, &index)) {
        return false;
    }

    const CosMutSpan span = mutator->spans[index];

    /*
     * Leave a space behind, so that the neighbouring tokens stay separate. One
     * time in eight, do not -- fusing two tokens into one is its own
     * interesting input.
     */
    static const unsigned char space = ' ';
    const bool fuse = cos_mut_random_chance_(&mutator->rng, 12);

    return cos_mut_buffer_splice_(&mutator->buffer,
                                  span.offset,
                                  span.length,
                                  fuse ? NULL : &space,
                                  fuse ? 0 : 1);
}

bool
cos_mut_op_number_boundary_(CosMutator *mutator)
{
    COS_IMPL_PARAM_CHECK(mutator != NULL);

    /*
     * Values at and past the edges of the integer widths libcos parses into,
     * plus forms that are not numbers at all but appear where one is expected.
     */
    static const char * const boundaries[] = {
        "0",
        "1",
        "-1",
        "2147483647",
        "2147483648",
        "-2147483648",
        "-2147483649",
        "4294967295",
        "65535",
        "65536",
        "9223372036854775807",
        "9223372036854775808",
        "18446744073709551616",
        "0000000001",
        "--1",
        "1e309",
        "0.0.0",
        ".",
        "+",
        "-",
    };

    size_t index = 0;
    if (!cos_mutator_pick_span_(mutator, COS_MUT_TYPE_MASK_NUMBER, &index)) {
        return false;
    }

    const CosMutSpan span = mutator->spans[index];

    char text[512];
    size_t text_length = 0;

    if (cos_mut_random_chance_(&mutator->rng, 50)) {
        /*
         * Nudge the existing value. Small deltas around a value the input
         * already uses reach off-by-one handling that a boundary constant,
         * which is far from any real value in the file, never would.
         */
        static const long deltas[] = {1, -1, 2, -2, 256, -256};

        long value = 0;
        for (uint32_t i = 0; i < span.length && i < 18; i++) {
            const unsigned char c = mutator->buffer.data[span.offset + i];
            if (c < '0' || c > '9') {
                break;
            }
            value = (value * 10) + (long)(c - '0');
        }

        const size_t pick = (size_t)cos_mut_random_below_(
            &mutator->rng,
            (uint32_t)(sizeof(deltas) / sizeof(deltas[0])));

        const int written = snprintf(text, sizeof(text), "%ld",
                                     value + deltas[pick]);
        if (written <= 0 || (size_t)written >= sizeof(text)) {
            return false;
        }
        text_length = (size_t)written;
    }
    else if (cos_mut_random_chance_(&mutator->rng, 10)) {
        /* A digit run far past any width the parser accumulates into. */
        text_length = 400;
        memset(text, '9', text_length);
    }
    else {
        const size_t pick = (size_t)cos_mut_random_below_(
            &mutator->rng,
            (uint32_t)(sizeof(boundaries) / sizeof(boundaries[0])));

        text_length = strlen(boundaries[pick]);
        memcpy(text, boundaries[pick], text_length);
    }

    return cos_mut_buffer_splice_(&mutator->buffer,
                                  span.offset,
                                  span.length,
                                  (const unsigned char *)text,
                                  text_length);
}

bool
cos_mut_op_keyword_retype_(CosMutator *mutator)
{
    COS_IMPL_PARAM_CHECK(mutator != NULL);

    /*
     * Siblings, not arbitrary replacements: swapping a keyword for one that can
     * legally appear in the same position keeps the mutant close enough to
     * valid that the parser gets past its structural checks and into the code
     * that acts on the keyword.
     */
    static const struct {
        CosMutKeyword keyword;
        const char *replacement;
    } siblings[] = {
        {CosMutKeyword_Obj, "endobj"},
        {CosMutKeyword_EndObj, "obj"},
        {CosMutKeyword_Stream, "endstream"},
        {CosMutKeyword_EndStream, "stream"},
        {CosMutKeyword_R, "obj"},
        {CosMutKeyword_N, "f"},
        {CosMutKeyword_F, "n"},
        {CosMutKeyword_True, "false"},
        {CosMutKeyword_False, "null"},
        {CosMutKeyword_Null, "true"},
        {CosMutKeyword_Trailer, "xref"},
        {CosMutKeyword_XRef, "trailer"},
        {CosMutKeyword_StartXRef, "startxrefx"},
    };

    /* Collect the keyword spans that have a defined sibling. */
    size_t candidates[64];
    size_t candidate_count = 0;

    for (size_t i = 0;
         i < mutator->span_count &&
         candidate_count < (sizeof(candidates) / sizeof(candidates[0]));
         i++) {
        if (mutator->spans[i].type != (uint16_t)CosMutSpanType_Keyword ||
            mutator->spans[i].keyword == (uint16_t)CosMutKeyword_None ||
            mutator->spans[i].keyword == (uint16_t)CosMutKeyword_Other) {
            continue;
        }
        candidates[candidate_count++] = i;
    }

    if (candidate_count == 0) {
        return false;
    }

    const size_t pick = candidates[cos_mut_random_below_(
        &mutator->rng,
        (uint32_t)candidate_count)];
    const CosMutSpan span = mutator->spans[pick];

    for (size_t i = 0; i < (sizeof(siblings) / sizeof(siblings[0])); i++) {
        if (siblings[i].keyword != (CosMutKeyword)span.keyword) {
            continue;
        }

        return cos_mut_buffer_splice_(&mutator->buffer,
                                      span.offset,
                                      span.length,
                                      (const unsigned char *)siblings[i].replacement,
                                      strlen(siblings[i].replacement));
    }

    return false;
}

bool
cos_mut_op_container_nest_(CosMutator *mutator)
{
    COS_IMPL_PARAM_CHECK(mutator != NULL);

    size_t index = 0;
    if (!cos_mutator_pick_span_(mutator, COS_MUT_TYPE_MASK_VALUE, &index)) {
        return false;
    }

    const CosMutSpan span = mutator->spans[index];

    /*
     * Draw the depth log-uniformly over 1..512 so that the shallow cases stay
     * common while runs still straddle whatever recursion limit the parser
     * enforces.
     */
    const uint32_t magnitude = cos_mut_random_below_(&mutator->rng, 10);
    uint32_t depth = 1u + cos_mut_random_below_(&mutator->rng, 1u << magnitude);

    const bool use_array = cos_mut_random_chance_(&mutator->rng, 50);
    const char * const open = use_array ? "[" : "<< /K ";
    const char * const close = use_array ? "]" : " >>";
    const size_t open_length = strlen(open);
    const size_t close_length = strlen(close);

    /* Bound the work by what the buffer could possibly hold. */
    const size_t per_level = open_length + close_length;
    if (mutator->buffer.max_length <= mutator->buffer.length) {
        return false;
    }
    const size_t headroom =
        (mutator->buffer.max_length - mutator->buffer.length) / per_level;
    if (headroom == 0) {
        return false;
    }
    if ((size_t)depth > headroom) {
        depth = (uint32_t)headroom;
    }

    /*
     * Build the wrapper around a staged copy of the span, then splice the whole
     * thing in as one edit.
     */
    if (!cos_mut_op_stage_(mutator, span.offset, span.length)) {
        return false;
    }

    for (uint32_t level = 0; level < depth; level++) {
        if (!cos_mut_buffer_splice_(&mutator->scratch,
                                    0,
                                    0,
                                    (const unsigned char *)open,
                                    open_length)) {
            return false;
        }
        if (!cos_mut_buffer_splice_(&mutator->scratch,
                                    mutator->scratch.length,
                                    0,
                                    (const unsigned char *)close,
                                    close_length)) {
            return false;
        }
    }

    return cos_mut_buffer_splice_(&mutator->buffer,
                                  span.offset,
                                  span.length,
                                  mutator->scratch.data,
                                  mutator->scratch.length);
}

COS_ASSUME_NONNULL_END
