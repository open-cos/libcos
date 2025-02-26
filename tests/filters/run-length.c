/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "CosTest.h"
#include "common/Assert.h"
#include "common/CosMacros.h"

#include <libcos/filters/CosRunLengthFilter.h>
#include <libcos/io/CosMemoryStream.h>

#include <stdio.h>
#include <string.h>

static bool
run_length_set_source(CosRunLengthFilter *run_length_filter,
                      char *input,
                      size_t input_size)
{
    COS_IMPL_PARAM_CHECK(run_length_filter != NULL);
    COS_IMPL_PARAM_CHECK(input != NULL);

    CosMemoryStream * const input_stream = cos_memory_stream_create(input,
                                                                    input_size,
                                                                    false);
    if (COS_UNLIKELY(!input_stream)) {
        return false;
    }

    cos_filter_attach_source((CosFilter *)run_length_filter,
                             (CosStream *)input_stream);

    return true;
}

typedef struct TestFixture {
    CosRunLengthFilter *run_length_filter;
} TestFixture;

static bool
setup(TestFixture *fixture)
{
    CosRunLengthFilter * const run_length_filter = cos_run_length_filter_create();
    if (COS_UNLIKELY(!run_length_filter)) {
        return false;
    }

    fixture->run_length_filter = run_length_filter;
    return true;
}

static void
teardown(TestFixture *fixture)
{
    if (fixture->run_length_filter) {
        cos_stream_close((CosStream *)fixture->run_length_filter);
        fixture->run_length_filter = NULL;
    }
}

TEST_CASE_BEGIN(run_length_hello_world_decode)
{
    char input[] = "12 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20";

    if (!run_length_set_source(fixture->run_length_filter,
                               input,
                               sizeof(input))) {
        TEST_FAILURE();
    }

    char output[256] = {0};
    size_t total_read_count = 0;

    while (total_read_count < sizeof(output) - 1) {
        const size_t read_count = cos_stream_read((CosStream *)fixture->run_length_filter,
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

    char expected_output[] = "3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 ";
    const size_t expected_output_length = sizeof(expected_output) - 1;

    if (total_read_count != expected_output_length ||
        memcmp(output,
               expected_output,
               COS_MIN(total_read_count, expected_output_length)) != 0) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}

TEST_CASE_END

TEST_MAIN()
{
    TestFixture fixture = {0};

    TEST_RUN(run_length_hello_world_decode, &fixture);

    return EXIT_SUCCESS;
}
