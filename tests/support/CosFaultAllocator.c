/*
 * Copyright (c) 2026 OpenCOS.
 */

#include "support/CosFaultAllocator.h"

#include <libcos/common/memory/CosAllocator.h>

#include <stdio.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosFaultAllocator {
    /** The wrapped allocator handed to the code under test. */
    CosAllocator *allocator;

    /** The 1-based request number to fail on, or 0 to disable injection. */
    size_t fail_at;

    /** Whether the fault, once reached, persists for later requests too. */
    CosFaultMode mode;

    /** Allocation requests seen since the last arm. */
    size_t call_count;

    /** Whether an injected failure has actually been returned since the arm. */
    bool triggered;

    /** Live allocations: +1 on success, -1 on free. */
    size_t outstanding;
};

// MARK: - Callbacks

static bool
cos_fault_should_fail_(CosFaultAllocator *fault_allocator)
{
    fault_allocator->call_count += 1;

    if (fault_allocator->fail_at == 0) {
        return false;
    }

    const bool reached = (fault_allocator->mode == COS_FAULT_PERSISTENT)
                             ? (fault_allocator->call_count >= fault_allocator->fail_at)
                             : (fault_allocator->call_count == fault_allocator->fail_at);
    if (reached) {
        fault_allocator->triggered = true;
    }
    return reached;
}

static void * COS_Nullable
cos_fault_alloc_(size_t size,
                 void * COS_Nullable user_data)
{
    CosFaultAllocator * const fault_allocator = user_data;

    if (cos_fault_should_fail_(fault_allocator)) {
        return NULL;
    }

    void * const ptr = malloc(size);
    if (ptr) {
        fault_allocator->outstanding += 1;
    }
    return ptr;
}

static void * COS_Nullable
cos_fault_realloc_(void * COS_Nullable ptr,
                   size_t size,
                   void * COS_Nullable user_data)
{
    CosFaultAllocator * const fault_allocator = user_data;

    if (cos_fault_should_fail_(fault_allocator)) {
        // A failed realloc leaves the original block allocated and untouched, so
        // the outstanding count is unchanged.
        return NULL;
    }

    void * const new_ptr = realloc(ptr, size);
    if (new_ptr && !ptr) {
        // Acted as an allocation of a fresh block.
        fault_allocator->outstanding += 1;
    }
    // A successful grow/shrink of an existing block is one block in, one out:
    // the outstanding count does not change.
    return new_ptr;
}

static void
cos_fault_dealloc_(void *ptr,
                   void * COS_Nullable user_data)
{
    CosFaultAllocator * const fault_allocator = user_data;

    fault_allocator->outstanding -= 1;
    free(ptr);
}

static const CosAllocatorCallbacks k_fault_callbacks = {
    .alloc = &cos_fault_alloc_,
    .realloc = &cos_fault_realloc_,
    .dealloc = &cos_fault_dealloc_,
    .retain = NULL,
    .release = NULL,
};

// MARK: - Lifecycle

CosFaultAllocator *
cos_fault_allocator_create(void)
{
    CosFaultAllocator * const fault_allocator = calloc(1, sizeof(CosFaultAllocator));
    if (!fault_allocator) {
        return NULL;
    }

    // The wrapped allocator is built off the default allocator, so its own
    // bookkeeping never runs through the callbacks above.
    CosAllocator * const allocator = cos_allocator_create(NULL,
                                                          &k_fault_callbacks,
                                                          fault_allocator);
    if (!allocator) {
        free(fault_allocator);
        return NULL;
    }

    fault_allocator->allocator = allocator;
    return fault_allocator;
}

void
cos_fault_allocator_destroy(CosFaultAllocator * COS_Nullable fault_allocator)
{
    if (!fault_allocator) {
        return;
    }

    cos_allocator_destroy(fault_allocator->allocator);
    free(fault_allocator);
}

CosAllocator *
cos_fault_allocator_get(CosFaultAllocator *fault_allocator)
{
    return fault_allocator->allocator;
}

void
cos_fault_allocator_arm(CosFaultAllocator *fault_allocator,
                        size_t fail_at,
                        CosFaultMode mode)
{
    fault_allocator->fail_at = fail_at;
    fault_allocator->mode = mode;
    fault_allocator->call_count = 0;
    fault_allocator->triggered = false;
}

bool
cos_fault_allocator_was_triggered(const CosFaultAllocator *fault_allocator)
{
    return fault_allocator->triggered;
}

size_t
cos_fault_allocator_total_allocs(const CosFaultAllocator *fault_allocator)
{
    return fault_allocator->call_count;
}

size_t
cos_fault_allocator_outstanding(const CosFaultAllocator *fault_allocator)
{
    return fault_allocator->outstanding;
}

// MARK: - OOM test driver

int
cos_oom_test_run(CosOomTestFunc func,
                 void * COS_Nullable ctx,
                 CosFaultMode mode)
{
    CosFaultAllocator * const fault_allocator = cos_fault_allocator_create();
    if (!fault_allocator) {
        (void)fprintf(stderr, "OOM test: could not create the fault allocator\n");
        return EXIT_FAILURE;
    }

    CosAllocator * const allocator = cos_fault_allocator_get(fault_allocator);

    int result = EXIT_FAILURE;

    // A generous bound so a broken "triggered implies failure" contract cannot
    // spin forever; a real operation makes far fewer allocations than this.
    const size_t max_iterations = 100000;

    for (size_t n = 1; n <= max_iterations; n++) {
        cos_fault_allocator_arm(fault_allocator, n, mode);

        const bool success = func(allocator, ctx);

        const bool triggered = cos_fault_allocator_was_triggered(fault_allocator);
        const size_t outstanding = cos_fault_allocator_outstanding(fault_allocator);

        if (outstanding != 0) {
            (void)fprintf(stderr,
                          "OOM test: leaked %zu block(s) at fail_at=%zu\n",
                          outstanding,
                          n);
            goto out;
        }

        if (triggered) {
            if (success) {
                (void)fprintf(stderr,
                              "OOM test: reported success despite injected failure at fail_at=%zu\n",
                              n);
                goto out;
            }
            // Graceful failure; move on to the next injection point.
            continue;
        }

        // No fault was injected this run, so the operation makes fewer than n
        // allocations: every allocation has now been failure-tested.
        if (!success) {
            (void)fprintf(stderr,
                          "OOM test: operation failed with no injected fault at fail_at=%zu\n",
                          n);
            goto out;
        }

        result = EXIT_SUCCESS;
        goto out;
    }

    (void)fprintf(stderr,
                  "OOM test: exceeded %zu iterations without completing\n",
                  max_iterations);

out:
    cos_fault_allocator_destroy(fault_allocator);
    return result;
}

COS_ASSUME_NONNULL_END
