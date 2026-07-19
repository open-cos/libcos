/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "CosTest.h"
#include "common/Assert.h"
#include "common/CosMacros.h"

#include <libcos/common/CosError.h>
#include <libcos/filters/CosASCIIHexFilter.h>
#include <libcos/filters/CosFilter.h>
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

TEST_CASE_BEGIN(decode_oddNibble_paddedWithZero)
{
    /* PDF spec section 7.3.4.2: a trailing odd digit "A" means "A0". */
    char input[] = "A>";

    if (!ascii_hex_set_source(fixture->hex_filter,
                              input,
                              sizeof(input))) {
        TEST_FAILURE();
    }

    unsigned char output[1] = {0};
    const size_t read_count = cos_stream_read((CosStream *)fixture->hex_filter,
                                              output,
                                              sizeof(output),
                                              NULL);

    if (read_count != 1 || output[0] != 0xA0) {
        TEST_FAILURE();
    }
}

TEST_CASE_END

static void
ascii_hex_set_eod_level(CosASCIIHexFilter *hex_filter,
                        CosStrictLevel level)
{
    const CosFilterOptions options = {
        .eod_strict_level = level,
        .diagnostic_handler = NULL,
    };
    cos_filter_set_options_((CosFilter *)hex_filter, &options);
}

TEST_CASE_BEGIN(missing_marker_tolerated_when_off)
{
    /* "48" encodes "H" but the ">" end-of-data marker is absent. At
     * CosStrictLevel_Off the source EOF is an implicit terminator. */
    char input[] = "48";

    ascii_hex_set_eod_level(fixture->hex_filter, CosStrictLevel_Off);

    if (!ascii_hex_set_source(fixture->hex_filter, input, sizeof(input) - 1)) {
        TEST_FAILURE();
    }

    unsigned char output[1] = {0};
    CosError error = {COS_ERROR_NONE, NULL};
    const size_t read_count = cos_stream_read((CosStream *)fixture->hex_filter,
                                              output,
                                              sizeof(output),
                                              &error);

    if (read_count != 1 || output[0] != 0x48 || error.code != COS_ERROR_NONE) {
        TEST_FAILURE();
    }
}
TEST_CASE_END

TEST_CASE_BEGIN(missing_marker_rejected_when_error)
{
    /* The same input, but at CosStrictLevel_Error the absent ">" marker fails
     * the decode with COS_ERROR_SYNTAX. */
    char input[] = "48";

    ascii_hex_set_eod_level(fixture->hex_filter, CosStrictLevel_Error);

    if (!ascii_hex_set_source(fixture->hex_filter, input, sizeof(input) - 1)) {
        TEST_FAILURE();
    }

    unsigned char output[1] = {0};
    CosError error = {COS_ERROR_NONE, NULL};
    const size_t read_count = cos_stream_read((CosStream *)fixture->hex_filter,
                                              output,
                                              sizeof(output),
                                              &error);

    if (read_count != 0 || error.code != COS_ERROR_SYNTAX) {
        TEST_FAILURE();
    }
}
TEST_CASE_END

TEST_MAIN()
{
    int (*tests_to_run[])(TestFixture *) = {
        &ascii_hex_hello_world_decode,
        &decode_oddNibble_paddedWithZero,
        &missing_marker_tolerated_when_off,
        &missing_marker_rejected_when_error,
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
