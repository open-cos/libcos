/*
 * Copyright (c) 2026 OpenCOS.
 */

#ifndef LIBCOS_TESTS_SUPPORT_COS_FAULT_ALLOCATOR_H
#define LIBCOS_TESTS_SUPPORT_COS_FAULT_ALLOCATOR_H

#include <libcos/common/CosDefines.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @file CosFaultAllocator.h
 *
 * A SQLite-style out-of-memory test driver. It installs a fault-injecting
 * allocator into the global memory allocator (see @c cos_set_memory_allocator ),
 * runs an operation once per allocation it makes -- failing the 1st, then the
 * 2nd, and so on -- and checks that each injected failure is handled cleanly and
 * without leaking. Test-only: linked into @c libcos-test , not the library.
 */

/**
 * How an injected fault behaves once the fail-at threshold is reached.
 */
typedef enum CosFaultMode {
    /** Only the Nth allocation fails; the (N+1)th and later succeed again. */
    COS_FAULT_TRANSIENT,

    /** The Nth allocation and every later one fail. */
    COS_FAULT_PERSISTENT,
} CosFaultMode;

/**
 * The operation exercised under OOM injection.
 *
 * @param ctx Caller context, or @c NULL.
 *
 * @return @c true only if the operation ran to completion with no allocation
 * failing; @c false if it aborted because an allocation returned @c NULL.
 */
typedef bool (*CosOomTestFunc)(void * COS_Nullable ctx);

/**
 * Runs @p func repeatedly under SQLite-style OOM injection.
 *
 * The fault allocator is installed globally for the duration and restored on
 * return. For @c n = 1, 2, 3, ... it is armed to fail the n-th allocation and
 * @p func is run. Each run must either fail gracefully (when the injected fault
 * was reached) or succeed (once @c n exceeds the number of allocations @p func
 * makes), and must never leak. The loop stops the first time @p func completes
 * without the fault being reached.
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
