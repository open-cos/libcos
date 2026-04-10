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

TEST_CASE_BEGIN(decode_eod_only)
{
    /* EOD marker alone produces no output. */
    char input[] = "\x80";

    if (!run_length_set_source(fixture->run_length_filter,
                               input,
                               sizeof(input) - 1)) {
        TEST_FAILURE();
    }

    char output[4] = {0};
    const size_t read_count = cos_stream_read((CosStream *)fixture->run_length_filter,
                                              output,
                                              sizeof(output),
                                              NULL);

    if (read_count != 0) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}
TEST_CASE_END

TEST_CASE_BEGIN(decode_literal_run)
{
    /* \x04 = literal indicator 4, copy next 5 bytes, then EOD. */
    char input[] = "\x04Hello\x80";

    if (!run_length_set_source(fixture->run_length_filter,
                               input,
                               sizeof(input) - 1)) {
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

    const char expected[] = "Hello";
    const size_t expected_length = sizeof(expected) - 1;

    if (total_read_count != expected_length ||
        memcmp(output, expected, expected_length) != 0) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}
TEST_CASE_END

TEST_CASE_BEGIN(decode_copy_run)
{
    /* \xFE = copy indicator 254, repeat next byte 257-254=3 times, then EOD. */
    char input[] = "\xFE*\x80";

    if (!run_length_set_source(fixture->run_length_filter,
                               input,
                               sizeof(input) - 1)) {
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

    const char expected[] = "***";
    const size_t expected_length = sizeof(expected) - 1;

    if (total_read_count != expected_length ||
        memcmp(output, expected, expected_length) != 0) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}
TEST_CASE_END

TEST_CASE_BEGIN(decode_mixed_runs)
{
    /* \x01 = literal run of 2 bytes ("Hi"), \xFD = repeat '!' 257-253=4 times, then EOD. */
    char input[] = "\x01Hi\xFD!\x80";

    if (!run_length_set_source(fixture->run_length_filter,
                               input,
                               sizeof(input) - 1)) {
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

    const char expected[] = "Hi!!!!";
    const size_t expected_length = sizeof(expected) - 1;

    if (total_read_count != expected_length ||
        memcmp(output, expected, expected_length) != 0) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}
TEST_CASE_END

TEST_MAIN()
{
    TestFixture fixture = {0};

    TEST_RUN(decode_eod_only, &fixture);
    TEST_RUN(decode_literal_run, &fixture);
    TEST_RUN(decode_copy_run, &fixture);
    TEST_RUN(decode_mixed_runs, &fixture);

    return EXIT_SUCCESS;
}
