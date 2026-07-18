/*
 * Copyright (c) 2026 OpenCOS.
 */

#ifndef LIBCOS_TESTS_FUZZ_COS_MUTATOR_REPAIR_H
#define LIBCOS_TESTS_FUZZ_COS_MUTATOR_REPAIR_H

#include "CosMutator.h"

#include <libcos/common/CosDefines.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef enum CosMutRepairFlags {
    CosMutRepairFlag_None = 0,
    CosMutRepairFlag_Header = 1 << 0,
    CosMutRepairFlag_Tail = 1 << 1, /* startxref and %%EOF */
    CosMutRepairFlag_All = 0x03,
} CosMutRepairFlags;

/** The number of distinct repair stages, for drawing a random flag mask. */
#define COS_MUT_REPAIR_STAGE_COUNT 2

/**
 * Rewrites the mutator's working buffer so that its file-level structure agrees
 * with its actual layout.
 *
 * Only the stages named in @p flags are applied, so that partially consistent
 * files keep occurring. Repair touches only the mutator's own output buffer:
 * committed corpus files are never rewritten, and a crash artifact remains a
 * plain byte-for-byte reproducible file.
 *
 * @return false when a stage would push the buffer past its cap, in which case
 * the buffer is restored to its pre-repair contents. Truncating a repaired
 * buffer would be worse than not repairing, since it would invalidate the very
 * offsets the repair just wrote.
 */
bool
cos_mut_repair_apply_(CosMutator *mutator,
                      CosMutRepairFlags flags);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_TESTS_FUZZ_COS_MUTATOR_REPAIR_H */
