/*
 * Copyright (c) 2026 OpenCOS.
 */

#include "support/CosFaultAllocator.h"

#include <libcos/common/memory/CosMemory.h>

#include <stdio.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

// The mutable state behind an installed fault allocator, passed as the
// allocator's user_data so the callbacks stay reentrant (no file-static state).
typedef struct CosFaultControl {
    /** The 1-based request number to fail on, or 0 to disable injection. */
    size_t fail_at;

    /** Whether the fault, once reached, persists for later requests too. */
    CosFaultMode mode;

    /** Allocation requests seen since the last arm. */
    size_t call_count;

    /** Whether any injected failure (benign or not) has fired since the arm. */
    bool injected;

    /**
     * Whether an injected failure fired outside a benign-malloc region since the
     * arm. Such a failure is expected to propagate to an operation-level error;
     * a failure inside a benign region may be absorbed instead.
     */
    bool non_benign_triggered;

    /** Live allocations: +1 on success, -1 on free. */
    size_t outstanding;
} CosFaultControl;

// MARK: - Fault allocator callbacks

static bool
cos_fault_should_fail_(CosFaultControl *control)
{
    control->call_count += 1;

    if (control->fail_at == 0) {
        return false;
    }

    return (control->mode == COS_FAULT_PERSISTENT)
               ? (control->call_count >= control->fail_at)
               : (control->call_count == control->fail_at);
}

// Records that an injected failure is about to be returned, classifying it by
// whether the current allocation sits inside a benign-malloc region.
static void
cos_fault_record_injection_(CosFaultControl *control)
{
    control->injected = true;
    if (!cos_memory_in_benign_region()) {
        control->non_benign_triggered = true;
    }
}

static void * COS_Nullable
cos_fault_malloc_(size_t size,
                  void * COS_Nullable user_data)
{
    CosFaultControl * const control = user_data;

    if (cos_fault_should_fail_(control)) {
        cos_fault_record_injection_(control);
        return NULL;
    }

    void * const ptr = malloc(size);
    if (ptr) {
        control->outstanding += 1;
    }
    return ptr;
}

static void * COS_Nullable
cos_fault_realloc_(void * COS_Nullable ptr,
                   size_t size,
                   void * COS_Nullable user_data)
{
    CosFaultControl * const control = user_data;

    if (cos_fault_should_fail_(control)) {
        cos_fault_record_injection_(control);
        // A failed realloc leaves the original block allocated and untouched, so
        // the outstanding count is unchanged.
        return NULL;
    }

    void * const new_ptr = realloc(ptr, size);
    if (new_ptr && !ptr) {
        // Acted as an allocation of a fresh block.
        control->outstanding += 1;
    }
    // A successful grow/shrink of an existing block is one block in, one out:
    // the outstanding count does not change.
    return new_ptr;
}

static void
cos_fault_free_(void *ptr,
                void * COS_Nullable user_data)
{
    CosFaultControl * const control = user_data;

    control->outstanding -= 1;
    free(ptr);
}

// MARK: - OOM test driver

int
cos_oom_test_run(CosOomTestFunc func,
                 void * COS_Nullable ctx,
                 CosFaultMode mode)
{
    CosFaultControl control = {
        .fail_at = 0,
        .mode = mode,
        .call_count = 0,
        .injected = false,
        .non_benign_triggered = false,
        .outstanding = 0,
    };

    const CosMemoryAllocator fault_allocator = {
        .malloc = &cos_fault_malloc_,
        .realloc = &cos_fault_realloc_,
        .free = &cos_fault_free_,
        .user_data = &control,
    };

    // Saved so the process's normal allocator is restored no matter how the loop
    // ends -- the fault allocator is only ever installed around func itself.
    const CosMemoryAllocator previous = cos_get_memory_allocator();

    int result = EXIT_FAILURE;

    // A generous bound so a broken "triggered implies failure" contract cannot
    // spin forever; a real operation makes far fewer allocations than this.
    const size_t max_iterations = 100000;

    for (size_t n = 1; n <= max_iterations; n++) {
        control.fail_at = n;
        control.call_count = 0;
        control.injected = false;
        control.non_benign_triggered = false;
        control.outstanding = 0;

        cos_set_memory_allocator(&fault_allocator);
        const bool success = func(ctx);
        cos_set_memory_allocator(&previous);

        if (control.outstanding != 0) {
            (void)fprintf(stderr,
                          "OOM test: leaked %zu block(s) at fail_at=%zu\n",
                          control.outstanding,
                          n);
            goto out;
        }

        if (!control.injected) {
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

        // A fault fired. A failure outside a benign region must propagate; a
        // benign one may be absorbed, so success is only a defect when a
        // non-benign failure was injected.
        if (control.non_benign_triggered && success) {
            (void)fprintf(stderr,
                          "OOM test: reported success despite non-benign injected failure at fail_at=%zu\n",
                          n);
            goto out;
        }

        // Otherwise: graceful failure, or a benign failure that was correctly
        // absorbed. Fall through to the next injection point.
    }

    (void)fprintf(stderr,
                  "OOM test: exceeded %zu iterations without completing\n",
                  max_iterations);

out:
    cos_set_memory_allocator(&previous);
    return result;
}

COS_ASSUME_NONNULL_END
