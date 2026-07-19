/*
 * Copyright (c) 2026 OpenCOS.
 */

#ifndef LIBCOS_TESTS_SUPPORT_COS_FAULT_ALLOCATOR_H
#define LIBCOS_TESTS_SUPPORT_COS_FAULT_ALLOCATOR_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @file CosFaultAllocator.h
 *
 * A @c CosAllocator that fails a chosen allocation on demand, plus a
 * SQLite-style out-of-memory test driver built on top of it. Test-only: this
 * is linked into @c libcos-test, not into the library.
 */

/**
 * How an armed fault behaves once the fail-at threshold is reached.
 */
typedef enum CosFaultMode {
    /** Only the Nth allocation fails; the (N+1)th and later succeed again. */
    COS_FAULT_TRANSIENT,

    /** The Nth allocation and every later one fail. */
    COS_FAULT_PERSISTENT,
} CosFaultMode;

typedef struct CosFaultAllocator CosFaultAllocator;

// MARK: - Fault allocator

/**
 * Creates a fault-injecting allocator.
 *
 * The allocator starts disarmed (no injected failures). Only allocations routed
 * through it are counted or failed; it wraps the C library allocator for the
 * requests it does let through.
 *
 * @return The fault allocator, or @c NULL on allocation failure.
 */
CosFaultAllocator * COS_Nullable
cos_fault_allocator_create(void);

/**
 * Destroys a fault allocator.
 *
 * @param fault_allocator The fault allocator, or @c NULL.
 */
void
cos_fault_allocator_destroy(CosFaultAllocator * COS_Nullable fault_allocator);

/**
 * The @c CosAllocator to hand to the code under test.
 *
 * @param fault_allocator The fault allocator.
 *
 * @return The wrapped allocator. Owned by @p fault_allocator; do not destroy it.
 */
CosAllocator *
cos_fault_allocator_get(CosFaultAllocator *fault_allocator);

/**
 * Arms the allocator to fail on the @p fail_at-th allocation request.
 *
 * Resets the call counter and the triggered flag, but not the outstanding
 * count, so a leak from a previous run is still visible.
 *
 * @param fault_allocator The fault allocator.
 * @param fail_at The 1-based request number to fail on, or @c 0 to disable
 * injection entirely.
 * @param mode Whether the failure is transient or persistent.
 */
void
cos_fault_allocator_arm(CosFaultAllocator *fault_allocator,
                        size_t fail_at,
                        CosFaultMode mode);

/**
 * Whether an injected failure has actually been returned since the last arm.
 *
 * @param fault_allocator The fault allocator.
 *
 * @return @c true if the armed failure was reached and injected.
 */
bool
cos_fault_allocator_was_triggered(const CosFaultAllocator *fault_allocator);

/**
 * The number of allocation requests seen since the last arm.
 *
 * @param fault_allocator The fault allocator.
 *
 * @return The request count.
 */
size_t
cos_fault_allocator_total_allocs(const CosFaultAllocator *fault_allocator);

/**
 * The number of live allocations: incremented on each successful allocation and
 * decremented on each free. A non-zero value after teardown means a leak.
 *
 * @param fault_allocator The fault allocator.
 *
 * @return The outstanding allocation count.
 */
size_t
cos_fault_allocator_outstanding(const CosFaultAllocator *fault_allocator);

// MARK: - OOM test driver

/**
 * The operation exercised under OOM injection.
 *
 * @param allocator The armed allocator to thread into whatever it builds.
 * @param ctx Caller context, or @c NULL.
 *
 * @return @c true only if the operation ran to completion with no allocation
 * failing; @c false if it aborted because an allocation returned @c NULL.
 */
typedef bool (*CosOomTestFunc)(CosAllocator *allocator,
                               void * COS_Nullable ctx);

/**
 * Runs @p func repeatedly under SQLite-style OOM injection.
 *
 * For @c n = 1, 2, 3, ... the allocator is armed to fail the n-th allocation
 * and @p func is run. Each run must either fail gracefully (when the injected
 * fault was reached) or succeed (once @c n exceeds the number of allocations
 * @p func makes), and must never leak. The loop stops the first time @p func
 * completes without the fault being reached.
 *
 * @param func The operation under test.
 * @param ctx Caller context passed through to @p func, or @c NULL.
 * @param mode Whether to inject transient or persistent faults.
 *
 * @return @c EXIT_SUCCESS if every iteration behaved correctly, otherwise
 * @c EXIT_FAILURE (with a diagnostic on @c stderr).
 */
int
cos_oom_test_run(CosOomTestFunc func,
                 void * COS_Nullable ctx,
                 CosFaultMode mode);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_TESTS_SUPPORT_COS_FAULT_ALLOCATOR_H */
