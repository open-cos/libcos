/*
 * Copyright (c) 2026 OpenCOS.
 */

#include "CosTest.h"

#include <libcos/common/memory/CosMemory.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

/*
 * A test-local allocator whose failures are switchable. While control->block is
 * set, malloc/realloc return NULL; clearing it (the stand-in for an
 * out-of-memory handler "freeing memory") lets the next attempt succeed. This is
 * simpler than the SQLite-style injector in CosFaultAllocator, which is a
 * different harness aimed at leak/propagation sweeps rather than the retry path.
 */
typedef struct TestAllocControl {
    /** When true, every malloc/realloc returns NULL. */
    bool block;

    /** How many times an installed handler has been entered. */
    unsigned int handler_calls;

    /** The @c attempt value the handler was last called with. */
    unsigned int last_attempt;

    /** Set by the reentrancy handler: whether its inner allocation got NULL. */
    bool inner_was_null;
} TestAllocControl;

static void * COS_Nullable
test_malloc_(size_t size,
             void * COS_Nullable user_data)
{
    TestAllocControl * const control = user_data;
    if (control->block) {
        return NULL;
    }
    return malloc(size);
}

static void * COS_Nullable
test_realloc_(void * COS_Nullable ptr,
              size_t size,
              void * COS_Nullable user_data)
{
    TestAllocControl * const control = user_data;
    if (control->block) {
        return NULL;
    }
    return realloc(ptr, size);
}

static void
test_free_(void * COS_Nullable ptr,
           COS_ATTR_UNUSED void * COS_Nullable user_data)
{
    free(ptr);
}

// MARK: - Handlers

// Frees memory (clears the block) and asks for one retry.
static bool
handler_unblock_(COS_ATTR_UNUSED size_t size,
                 unsigned int attempt,
                 void * COS_Nullable user_data)
{
    TestAllocControl * const control = user_data;
    control->handler_calls += 1;
    control->last_attempt = attempt;
    control->block = false;
    return true;
}

// Reclaims nothing and gives up.
static bool
handler_give_up_(COS_ATTR_UNUSED size_t size,
                 COS_ATTR_UNUSED unsigned int attempt,
                 void * COS_Nullable user_data)
{
    TestAllocControl * const control = user_data;
    control->handler_calls += 1;
    return false;
}

// Allocates from within the handler to prove the reentrancy guard: the inner,
// still-blocked allocation must fail without re-entering the handler. Then it
// unblocks so the outer retry can succeed.
static bool
handler_reenter_(COS_ATTR_UNUSED size_t size,
                 COS_ATTR_UNUSED unsigned int attempt,
                 void * COS_Nullable user_data)
{
    TestAllocControl * const control = user_data;
    control->handler_calls += 1;

    void * const inner = cos_malloc(8);
    control->inner_was_null = (inner == NULL);
    cos_free(inner);

    control->block = false;
    return true;
}

// MARK: - Fixture

static CosMemoryAllocator
test_allocator_(TestAllocControl *control)
{
    const CosMemoryAllocator allocator = {
        .malloc = &test_malloc_,
        .realloc = &test_realloc_,
        .free = &test_free_,
        .user_data = control,
    };
    return allocator;
}

// MARK: - Tests

static int
retry_afterHandlerFreesMemory_Succeeds(void)
{
    TestAllocControl control = {.block = true};
    const CosMemoryAllocator previous = cos_get_memory_allocator();
    const CosMemoryAllocator allocator = test_allocator_(&control);

    cos_set_memory_allocator(&allocator);
    cos_set_out_of_memory_handler(&handler_unblock_, &control);

    void * const ptr = cos_malloc(16);
    const bool ok = (ptr != NULL)
                    && (control.handler_calls == 1)
                    && (control.last_attempt == 1);
    cos_free(ptr);

    cos_set_out_of_memory_handler(NULL, NULL);
    cos_set_memory_allocator(&previous);

    TEST_EXPECT(ok);
    return EXIT_SUCCESS;
}

static int
retry_whenHandlerGivesUp_ReturnsNull(void)
{
    TestAllocControl control = {.block = true};
    const CosMemoryAllocator previous = cos_get_memory_allocator();
    const CosMemoryAllocator allocator = test_allocator_(&control);

    cos_set_memory_allocator(&allocator);
    cos_set_out_of_memory_handler(&handler_give_up_, &control);

    void * const ptr = cos_malloc(16);
    const bool ok = (ptr == NULL) && (control.handler_calls == 1);
    cos_free(ptr);

    cos_set_out_of_memory_handler(NULL, NULL);
    cos_set_memory_allocator(&previous);

    TEST_EXPECT(ok);
    return EXIT_SUCCESS;
}

static int
retry_withNoHandler_ReturnsNullImmediately(void)
{
    TestAllocControl control = {.block = true};
    const CosMemoryAllocator previous = cos_get_memory_allocator();
    const CosMemoryAllocator allocator = test_allocator_(&control);

    cos_set_memory_allocator(&allocator);
    // No handler installed: the default is an immediate NULL.
    cos_set_out_of_memory_handler(NULL, NULL);

    void * const ptr = cos_malloc(16);
    const bool ok = (ptr == NULL) && (control.handler_calls == 0);
    cos_free(ptr);

    cos_set_memory_allocator(&previous);

    TEST_EXPECT(ok);
    return EXIT_SUCCESS;
}

static int
retry_handlerThatAllocates_DoesNotReenter(void)
{
    TestAllocControl control = {.block = true};
    const CosMemoryAllocator previous = cos_get_memory_allocator();
    const CosMemoryAllocator allocator = test_allocator_(&control);

    cos_set_memory_allocator(&allocator);
    cos_set_out_of_memory_handler(&handler_reenter_, &control);

    void * const ptr = cos_malloc(16);
    // The handler ran exactly once (its inner allocation did not re-enter it),
    // that inner allocation failed, and the outer retry then succeeded.
    const bool ok = (ptr != NULL)
                    && (control.handler_calls == 1)
                    && control.inner_was_null;
    cos_free(ptr);

    cos_set_out_of_memory_handler(NULL, NULL);
    cos_set_memory_allocator(&previous);

    TEST_EXPECT(ok);
    return EXIT_SUCCESS;
}

static int
retry_reallocPath_Succeeds(void)
{
    TestAllocControl control = {.block = false};
    const CosMemoryAllocator previous = cos_get_memory_allocator();
    const CosMemoryAllocator allocator = test_allocator_(&control);

    cos_set_memory_allocator(&allocator);

    // Allocate a real block first, then make the grow fail so the handler runs.
    void * const ptr = cos_malloc(16);
    bool ok = (ptr != NULL);

    control.block = true;
    control.handler_calls = 0;
    cos_set_out_of_memory_handler(&handler_unblock_, &control);

    void * const grown = cos_realloc(ptr, 64);
    ok = ok && (grown != NULL) && (control.handler_calls == 1);
    cos_free(grown);

    cos_set_out_of_memory_handler(NULL, NULL);
    cos_set_memory_allocator(&previous);

    TEST_EXPECT(ok);
    return EXIT_SUCCESS;
}

TEST_MAIN()
{
    TEST_EXPECT(retry_afterHandlerFreesMemory_Succeeds() == EXIT_SUCCESS);
    TEST_EXPECT(retry_whenHandlerGivesUp_ReturnsNull() == EXIT_SUCCESS);
    TEST_EXPECT(retry_withNoHandler_ReturnsNullImmediately() == EXIT_SUCCESS);
    TEST_EXPECT(retry_handlerThatAllocates_DoesNotReenter() == EXIT_SUCCESS);
    TEST_EXPECT(retry_reallocPath_Succeeds() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END
