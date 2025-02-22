/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "CosTest.h"
#include "common/Assert.h"
#include "common/CosMacros.h"

#include <libcos/filters/CosASCIIHexFilter.h>
#include <libcos/io/CosMemoryStream.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool
ascii_hex_set_source(CosASCIIHexFilter *ascii_hex_filter,
                     char *input,
                     size_t input_size)
{
    COS_IMPL_PARAM_CHECK(ascii_hex_filter != NULL);
    COS_IMPL_PARAM_CHECK(input != NULL);

    CosMemoryStream * const input_stream = cos_memory_stream_create(input,
                                                                    input_size,
                                                                    false);
    if (COS_UNLIKELY(!input_stream)) {
        return false;
    }

    cos_filter_attach_source((CosFilter *)ascii_hex_filter,
                             (CosStream *)input_stream);

    return true;
}

typedef struct TestFixture {
    CosASCIIHexFilter *hex_filter;
} TestFixture;

static bool
setup(TestFixture *fixture)
{
    CosASCIIHexFilter * const hex_filter = cos_ascii_hex_filter_create();
    if (COS_UNLIKELY(!hex_filter)) {
        return false;
    }

    fixture->hex_filter = hex_filter;
    return true;
}

static void
teardown(TestFixture *fixture)
{
    if (fixture->hex_filter) {
        cos_stream_close((CosStream *)fixture->hex_filter);
        fixture->hex_filter = NULL;
    }
}

TEST_CASE_BEGIN(ascii_hex_hello_world_decode)
{
    char input[] = "48 65 6C 6C 6F 2C 20 57 6F 72 6C 64 21";

    if (!ascii_hex_set_source(fixture->hex_filter,
                              input,
                              sizeof(input))) {
        TEST_FAILURE();
    }

    char output[256] = {0};
    size_t total_read_count = 0;

    while (total_read_count < sizeof(output) - 1) {
        const size_t read_count = cos_stream_read((CosStream *)fixture->hex_filter,
                                                  output + total_read_count,
                                                  sizeof(output) - 1 - total_read_count,
                                                  NULL);
        if (read_count == 0) {
            break;
        }
        total_read_count += read_count;
    }

    // Print up to 256 characters of the output.
    printf("Output: %.*s\n", (int)total_read_count, output);

    char expected_output[] = "Hello, World!";
    const size_t expected_output_length = sizeof(expected_output) - 1;

    if (total_read_count != expected_output_length ||
        memcmp(output,
               expected_output,
               COS_MIN(total_read_count, expected_output_length)) != 0) {
        TEST_FAILURE();
    }
}

TEST_CASE_END

TEST_MAIN()
{
    int (*tests_to_run[])(TestFixture *) = {
        &ascii_hex_hello_world_decode,
    };
    for (size_t i = 0; i < sizeof(tests_to_run) / sizeof(tests_to_run[0]); i++) {
        TestFixture fixture = {0};
        if (!setup(&fixture)) {
            return EXIT_FAILURE;
        }
        const int result = tests_to_run[i](&fixture);
        teardown(&fixture);
        if (result != EXIT_SUCCESS) {
            return result;
        }
    }

    return EXIT_SUCCESS;
}
