/*
 * Copyright (c) 2026 OpenCOS.
 */

#include "CosMutatorRepair.h"

#include "CosMutator-Private.h"
#include "common/Assert.h"

#include <stdio.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

/* "%PDF-1.7\n" -- the version libcos parses as major 1, minor 7. */
static const unsigned char cos_mut_repair_header_[] = {
    '%', 'P', 'D', 'F', '-', '1', '.', '7', '\n'};

/**
 * Stage 1: give the mutant a header cos_parser_parse_header_ accepts.
 *
 * This runs first because it can shift every later offset, and because phase 1
 * of cos_parser_parse gates the other two.
 */
static bool
cos_mut_repair_header_stage_(CosMutator *mutator)
{
    COS_IMPL_PARAM_CHECK(mutator != NULL);

    if (mutator->view.has_pdf_header) {
        return true;
    }

    /*
     * Prepend rather than overwrite. A mutant whose first bytes merely resemble
     * a header still has content there worth keeping as body junk, and
     * prepending is the one edit that cannot lose bytes.
     */
    return cos_mut_buffer_splice_(&mutator->buffer,
                                  0,
                                  0,
                                  cos_mut_repair_header_,
                                  sizeof(cos_mut_repair_header_));
}

/**
 * Stage 2: rewrite the tail so that the startxref offset names the xref
 * section.
 *
 * The whole tail is replaced rather than just the offset digits. That is the
 * only way to guarantee the exact shape cos_parser_find_startxref_ demands: it
 * scans the last 1024 bytes for the last "%%EOF", then walks backwards
 * accepting only space, tab, CR and LF as separators before requiring the nine
 * bytes "startxref". A stray comment or NUL anywhere in that region -- which
 * whitespace and delete operators readily produce -- fails the scan. Nothing
 * references the bytes past the xref section, so replacing them is safe and
 * needs no re-lex.
 */
static bool
cos_mut_repair_tail_stage_(CosMutator *mutator)
{
    COS_IMPL_PARAM_CHECK(mutator != NULL);

    const CosMutFileView * const view = &mutator->view;

    /*
     * Without an xref keyword there is no meaningful offset to point at, and
     * inventing one would only trade a phase 2 failure for a phase 3 failure.
     */
    if (!view->has_xref_kw) {
        return true;
    }

    char tail[64];
    const int written = snprintf(tail,
                                 sizeof(tail),
                                 "\nstartxref\n%lu\n%%%%EOF\n",
                                 (unsigned long)view->xref_kw_offset);
    if (written <= 0 || (size_t)written >= sizeof(tail)) {
        return false;
    }

    /*
     * Drop everything from the last startxref keyword onward, so that the
     * canonical tail is the only one in the scan window. With no startxref
     * present, append instead.
     */
    const size_t tail_start = view->has_startxref_kw
                                  ? (size_t)view->startxref_kw_offset
                                  : mutator->buffer.length;

    if (tail_start > mutator->buffer.length) {
        return false;
    }

    return cos_mut_buffer_splice_(&mutator->buffer,
                                  tail_start,
                                  mutator->buffer.length - tail_start,
                                  (const unsigned char *)tail,
                                  (size_t)written);
}

bool
cos_mut_repair_apply_(CosMutator *mutator,
                      CosMutRepairFlags flags)
{
    COS_IMPL_PARAM_CHECK(mutator != NULL);

    if (flags == CosMutRepairFlag_None || mutator->buffer.length == 0) {
        return true;
    }

    /* Snapshot, so that a stage that would overflow the cap can be undone. */
    cos_mut_buffer_set_max_(&mutator->snapshot, mutator->buffer.max_length);
    if (!cos_mut_buffer_assign_(&mutator->snapshot,
                                mutator->buffer.data,
                                mutator->buffer.length)) {
        return false;
    }

    if ((flags & CosMutRepairFlag_Header) != 0) {
        if (!cos_mutator_relex_(mutator) ||
            !cos_mut_repair_header_stage_(mutator)) {
            goto restore;
        }
    }

    if ((flags & CosMutRepairFlag_Tail) != 0) {
        /* The header stage may have shifted every offset. */
        if (!cos_mutator_relex_(mutator) ||
            !cos_mut_repair_tail_stage_(mutator)) {
            goto restore;
        }
    }

    return true;

restore:
    (void)cos_mut_buffer_assign_(&mutator->buffer,
                                 mutator->snapshot.data,
                                 mutator->snapshot.length);
    return false;
}

COS_ASSUME_NONNULL_END
