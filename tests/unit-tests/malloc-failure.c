/*
 * Copyright (c) 2026 OpenCOS.
 */

#include "CosTest.h"
#include "support/CosFaultAllocator.h"

#include <libcos/common/CosArray.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

/*
 * The operation under test: build a CosArray, grow it past its initial capacity
 * (forcing a resize), read an item back, and tear it down. Every allocation --
 * the array struct, its data buffer, and the resize -- routes through the global
 * cos_* functions, so each is a distinct injection point while the fault
 * allocator is installed.
 *
 * It returns false the moment any allocation fails, always destroying whatever
 * it managed to create so the fault allocator's outstanding count returns to 0.
 */
static bool
oom_build_and_grow_array_(void * COS_Nullable ctx)
{
    (void)ctx;

    CosArray * const array = cos_array_create(sizeof(int), NULL, 4);
    if (!array) {
        return false;
    }

    bool ok = true;

    // 32 items forces at least one resize past the initial capacity of 4.
    for (int i = 0; i < 32; i++) {
        if (!cos_array_append_item(array, &i, NULL)) {
            ok = false;
            break;
        }
    }

    if (ok) {
        int value = -1;
        if (!cos_array_get_item(array, 10, &value, NULL) || value != 10) {
            ok = false;
        }
    }

    cos_array_destroy(array);
    return ok;
}

static int
test_array_oom_transient(void)
{
    return cos_oom_test_run(oom_build_and_grow_array_, NULL, COS_FAULT_TRANSIENT);
}

static int
test_array_oom_persistent(void)
{
    return cos_oom_test_run(oom_build_and_grow_array_, NULL, COS_FAULT_PERSISTENT);
}

TEST_MAIN()
{
    TEST_EXPECT(test_array_oom_transient() == EXIT_SUCCESS);
    TEST_EXPECT(test_array_oom_persistent() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END
