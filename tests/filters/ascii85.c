/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "CosTest.h"
#include "common/Assert.h"
#include "common/CosMacros.h"

#include <libcos/filters/CosASCII85Filter.h>
#include <libcos/io/CosMemoryStream.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool
ascii85_set_source(CosASCII85Filter *ascii85_filter,
                   char *input,
                   size_t input_size)
{
    COS_IMPL_PARAM_CHECK(ascii85_filter != NULL);
    COS_IMPL_PARAM_CHECK(input != NULL);

    CosMemoryStream * const input_stream = cos_memory_stream_create(input,
                                                                    input_size,
                                                                    false);
    if (COS_UNLIKELY(!input_stream)) {
        return false;
    }

    cos_filter_attach_source((CosFilter *)ascii85_filter,
                             (CosStream *)input_stream);

    return true;
}

typedef struct TestFixture {
    CosASCII85Filter *ascii85_filter;
} TestFixture;

static bool
setup(TestFixture *fixture)
{
    CosASCII85Filter * const ascii85_filter = cos_ascii85_filter_create();
    if (COS_UNLIKELY(!ascii85_filter)) {
        return false;
    }

    fixture->ascii85_filter = ascii85_filter;
    return true;
}

static void
teardown(TestFixture *fixture)
{
    if (fixture->ascii85_filter) {
        cos_stream_close((CosStream *)fixture->ascii85_filter);
        fixture->ascii85_filter = NULL;
    }
}

TEST_CASE_BEGIN(decode_alphabet)
{
    char input[] = "@:E_WAS,RgBkhF\"D/O92EH6,BF`qtRH$T~>";

    if (!ascii85_set_source(fixture->ascii85_filter,
                            input,
                            sizeof(input) - 1)) {
        TEST_FAILURE();
    }

    char output[256] = {0};
    size_t total_read_count = 0;

    while (total_read_count < sizeof(output) - 1) {
        const size_t read_count = cos_stream_read((CosStream *)fixture->ascii85_filter,
                                                  output + total_read_count,
                                                  sizeof(output) - 1 - total_read_count,
                                                  NULL);
        if (read_count == 0) {
            break;
        }
        total_read_count += read_count;
    }

    const char expected[] = "abcdefghijklmnopqrstuvwxyz";
    const size_t expected_length = sizeof(expected) - 1;

    if (total_read_count != expected_length ||
        memcmp(output, expected, expected_length) != 0) {
        TEST_FAILURE();
    }
}
TEST_CASE_END

TEST_CASE_BEGIN(decode_z_group)
{
    /* 'z' is shorthand for four zero bytes. */
    char input[] = "z~>";

    if (!ascii85_set_source(fixture->ascii85_filter,
                            input,
                            sizeof(input) - 1)) {
        TEST_FAILURE();
    }

    unsigned char output[4] = {1, 1, 1, 1};
    const size_t read_count = cos_stream_read((CosStream *)fixture->ascii85_filter,
                                              output,
                                              sizeof(output),
                                              NULL);

    const unsigned char expected[4] = {0x00, 0x00, 0x00, 0x00};

    if (read_count != 4 ||
        memcmp(output, expected, sizeof(expected)) != 0) {
        TEST_FAILURE();
    }
}
TEST_CASE_END

TEST_CASE_BEGIN(decode_partial_block)
{
    /* PDF spec: a partial final block of n encoded chars represents n-1 decoded bytes.
     * "88/" encodes "Hi" (0x48, 0x69). Exercises the partial final block handling. */
    char input[] = "88/~>";
    const unsigned char expected[] = {0x48, 0x69}; // "Hi"

    if (!ascii85_set_source(fixture->ascii85_filter,
                            input,
                            sizeof(input) - 1)) {
        TEST_FAILURE();
    }

    unsigned char output[2] = {0};
    const size_t read_count = cos_stream_read((CosStream *)fixture->ascii85_filter,
                                              output,
                                              sizeof(output),
                                              NULL);

    if (read_count != sizeof(expected) ||
        memcmp(output, expected, read_count) != 0) {
        TEST_FAILURE();
    }
}
TEST_CASE_END

TEST_MAIN()
{
    TestFixture fixture = {0};
    TEST_RUN(decode_alphabet, &fixture);
    TEST_RUN(decode_z_group, &fixture);
    TEST_RUN(decode_partial_block, &fixture);

    return EXIT_SUCCESS;
}
