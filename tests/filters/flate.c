/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "CosTest.h"
#include "common/Assert.h"

#include <libcos/common/CosError.h>
#include <libcos/filters/CosFlateFilter.h>
#include <libcos/io/CosMemoryStream.h>

#include <stdint.h>
#include <string.h>

// Test vectors produced with Python's zlib and verified against zlib.decompress.

// clang-format off

// Level-0 (stored block) compression of "Hello, stored world!".
static const unsigned char stored_compressed[] = {
    0x78, 0x01, 0x01, 0x14, 0x00, 0xeb, 0xff, 0x48, 0x65, 0x6c, 0x6c, 0x6f,
    0x2c, 0x20, 0x73, 0x74, 0x6f, 0x72, 0x65, 0x64, 0x20, 0x77, 0x6f, 0x72,
    0x6c, 0x64, 0x21, 0x4c, 0x32, 0x07, 0x3b,
};
static const char stored_plain[] = "Hello, stored world!";

// Z_FIXED-strategy compression exercising the fixed Huffman + match path.
static const unsigned char fixed_compressed[] = {
    0x78, 0x01, 0x4b, 0x4c, 0x4a, 0x44, 0x81, 0xc9, 0x29, 0x08, 0x18, 0xef,
    0xe8, 0xe4, 0xec, 0xe2, 0xea, 0xe6, 0x1e, 0x0f, 0x91, 0x02, 0x00, 0x61,
    0x4e, 0x0f, 0xa6,
};
static const char fixed_plain[] = "ababababababababcdcdcdcdcdcd_ABCDEFG_ababab";

// Default (dynamic Huffman) compression of a longer, varied payload.
static const unsigned char dynamic_compressed[] = {
    0x78, 0xda, 0x0b, 0xc9, 0x48, 0x55, 0x28, 0x2c, 0xcd, 0x4c, 0xce, 0x56,
    0x48, 0x2a, 0xca, 0x2f, 0xcf, 0x53, 0x48, 0xcb, 0xaf, 0x50, 0xc8, 0x2a,
    0xcd, 0x2d, 0x28, 0x56, 0xc8, 0x2f, 0x4b, 0x2d, 0x52, 0x28, 0x01, 0x4a,
    0xe7, 0x24, 0x56, 0x55, 0x2a, 0xa4, 0xe4, 0xa7, 0xeb, 0x29, 0x84, 0x8c,
    0x2a, 0x26, 0x57, 0x31, 0x03, 0x23, 0x13, 0x33, 0x0b, 0x2b, 0x1b, 0x3b,
    0x07, 0x27, 0x17, 0x37, 0x0f, 0x2f, 0x1f, 0xbf, 0x80, 0xa0, 0x90, 0xb0,
    0x88, 0xa8, 0x98, 0xb8, 0x84, 0xa4, 0x94, 0xb4, 0x8c, 0xac, 0x9c, 0xbc,
    0x82, 0xa2, 0x92, 0xb2, 0x8a, 0xaa, 0x9a, 0xba, 0x86, 0xa6, 0x96, 0xb6,
    0x8e, 0xae, 0x9e, 0xbe, 0x81, 0xa1, 0x91, 0xb1, 0x89, 0xa9, 0x99, 0xb9,
    0x85, 0xa5, 0x95, 0xb5, 0x8d, 0xad, 0x9d, 0xbd, 0x83, 0xa3, 0x93, 0xb3,
    0x8b, 0xab, 0x9b, 0xbb, 0x87, 0xa7, 0x97, 0xb7, 0x8f, 0xaf, 0x9f, 0x7f,
    0x40, 0x60, 0x50, 0x70, 0x48, 0x68, 0x58, 0x78, 0x44, 0x64, 0x54, 0x74,
    0x4c, 0x6c, 0x5c, 0x7c, 0x42, 0x62, 0x52, 0x72, 0x4a, 0x6a, 0x5a, 0x7a,
    0x46, 0x66, 0x56, 0x76, 0x4e, 0x6e, 0x5e, 0x7e, 0x41, 0x61, 0x51, 0x71,
    0x49, 0x69, 0x59, 0x79, 0x45, 0x65, 0x55, 0x75, 0x4d, 0x6d, 0x5d, 0x7d,
    0x43, 0x63, 0x53, 0x73, 0x4b, 0x6b, 0x5b, 0x7b, 0x47, 0x67, 0x57, 0x77,
    0x4f, 0x6f, 0x5f, 0xff, 0x84, 0x89, 0x93, 0x26, 0x4f, 0x99, 0x3a, 0x6d,
    0xfa, 0x8c, 0x99, 0xb3, 0x66, 0xcf, 0x99, 0x3b, 0x6f, 0xfe, 0x82, 0x85,
    0x8b, 0x16, 0x2f, 0x59, 0xba, 0x6c, 0xf9, 0x8a, 0x95, 0xab, 0x56, 0xaf,
    0x59, 0xbb, 0x6e, 0xfd, 0x86, 0x8d, 0x9b, 0x36, 0x6f, 0xd9, 0xba, 0x6d,
    0xfb, 0x8e, 0x9d, 0xbb, 0x76, 0xef, 0xd9, 0xbb, 0x6f, 0xff, 0x81, 0x83,
    0x87, 0x0e, 0x1f, 0x39, 0x7a, 0xec, 0xf8, 0x89, 0x93, 0xa7, 0x4e, 0x9f,
    0x39, 0x7b, 0xee, 0xfc, 0x85, 0x8b, 0x97, 0x2e, 0x5f, 0xb9, 0x7a, 0xed,
    0xfa, 0x8d, 0x9b, 0xb7, 0x6e, 0xdf, 0xb9, 0x7b, 0xef, 0xfe, 0x83, 0x87,
    0x8f, 0x1e, 0x3f, 0x79, 0xfa, 0xec, 0xf9, 0x8b, 0x97, 0xaf, 0x5e, 0xbf,
    0x79, 0xfb, 0xee, 0xfd, 0x87, 0x8f, 0x9f, 0x3e, 0x7f, 0xf9, 0xfa, 0xed,
    0xfb, 0x8f, 0x9f, 0xbf, 0x7e, 0xff, 0xf9, 0xfb, 0xef, 0x3f, 0x00, 0x52,
    0xc5, 0x00, 0xc8,
};

// clang-format on

// Builds the payload that `dynamic_compressed` decodes to: eight copies of a
// sentence followed by every byte value 0x00..0xff.
static size_t
build_dynamic_plain(unsigned char *out, size_t out_size)
{
    static const char sentence[] = "The quick brown fox jumps over the lazy dog. ";
    const size_t sentence_len = sizeof(sentence) - 1;

    size_t pos = 0;
    for (int i = 0; i < 8; i++) {
        COS_ASSERT(pos + sentence_len <= out_size, "buffer too small");
        memcpy(out + pos, sentence, sentence_len);
        pos += sentence_len;
    }
    for (int b = 0; b < 256; b++) {
        COS_ASSERT(pos < out_size, "buffer too small");
        out[pos++] = (unsigned char)b;
    }
    return pos;
}

typedef struct TestFixture {
    CosFlateFilter *flate_filter;
} TestFixture;

static bool
setup(TestFixture *fixture)
{
    CosFlateFilter * const flate_filter = cos_flate_filter_create();
    if (COS_UNLIKELY(!flate_filter)) {
        return false;
    }

    fixture->flate_filter = flate_filter;
    return true;
}

static void
teardown(TestFixture *fixture)
{
    if (fixture->flate_filter) {
        cos_stream_close((CosStream *)fixture->flate_filter);
        fixture->flate_filter = NULL;
    }
}

static bool
flate_set_source(CosFlateFilter *flate_filter,
                 const unsigned char *input,
                 size_t input_size)
{
    CosMemoryStream * const input_stream =
        cos_memory_stream_create_readonly(input, input_size);
    if (COS_UNLIKELY(!input_stream)) {
        return false;
    }

    cos_filter_attach_source((CosFilter *)flate_filter,
                             (CosStream *)input_stream);
    return true;
}

// Reads the whole filter output into `output`, returning the total byte count.
// `chunk` bounds each read so small values exercise the buffer-refill path.
static size_t
flate_read_all(CosFlateFilter *flate_filter,
               unsigned char *output,
               size_t output_size,
               size_t chunk,
               CosError *out_error)
{
    size_t total = 0;
    while (total < output_size) {
        size_t want = output_size - total;
        if (want > chunk) {
            want = chunk;
        }
        const size_t read_count = cos_stream_read((CosStream *)flate_filter,
                                                  output + total,
                                                  want,
                                                  out_error);
        if (read_count == 0) {
            break;
        }
        total += read_count;
    }
    return total;
}

TEST_CASE_BEGIN(decode_stored_block)
{
    if (!flate_set_source(fixture->flate_filter,
                          stored_compressed,
                          sizeof(stored_compressed))) {
        TEST_FAILURE();
    }

    unsigned char output[64] = {0};
    const size_t plain_len = sizeof(stored_plain) - 1;
    const size_t total = flate_read_all(fixture->flate_filter,
                                        output,
                                        sizeof(output),
                                        sizeof(output),
                                        NULL);

    if (total != plain_len || memcmp(output, stored_plain, plain_len) != 0) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}

TEST_CASE_END

TEST_CASE_BEGIN(decode_fixed_huffman)
{
    if (!flate_set_source(fixture->flate_filter,
                          fixed_compressed,
                          sizeof(fixed_compressed))) {
        TEST_FAILURE();
    }

    unsigned char output[128] = {0};
    const size_t plain_len = sizeof(fixed_plain) - 1;
    const size_t total = flate_read_all(fixture->flate_filter,
                                        output,
                                        sizeof(output),
                                        sizeof(output),
                                        NULL);

    if (total != plain_len || memcmp(output, fixed_plain, plain_len) != 0) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}

TEST_CASE_END

TEST_CASE_BEGIN(decode_dynamic_huffman)
{
    unsigned char expected[1024];
    const size_t expected_len = build_dynamic_plain(expected, sizeof(expected));

    if (!flate_set_source(fixture->flate_filter,
                          dynamic_compressed,
                          sizeof(dynamic_compressed))) {
        TEST_FAILURE();
    }

    unsigned char output[1024] = {0};
    const size_t total = flate_read_all(fixture->flate_filter,
                                        output,
                                        sizeof(output),
                                        sizeof(output),
                                        NULL);

    if (total != expected_len || memcmp(output, expected, expected_len) != 0) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}

TEST_CASE_END

// The same dynamic payload, drained 7 bytes at a time, exercises the streaming
// state machine suspending and resuming across the filter's 256-byte buffer.
TEST_CASE_BEGIN(decode_dynamic_small_reads)
{
    unsigned char expected[1024];
    const size_t expected_len = build_dynamic_plain(expected, sizeof(expected));

    if (!flate_set_source(fixture->flate_filter,
                          dynamic_compressed,
                          sizeof(dynamic_compressed))) {
        TEST_FAILURE();
    }

    unsigned char output[1024] = {0};
    const size_t total = flate_read_all(fixture->flate_filter,
                                        output,
                                        sizeof(output),
                                        7,
                                        NULL);

    if (total != expected_len || memcmp(output, expected, expected_len) != 0) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}

TEST_CASE_END

// A truncated stream must stop short and report a syntax error rather than
// silently returning a complete-looking result.
TEST_CASE_BEGIN(decode_truncated_reports_error)
{
    if (!flate_set_source(fixture->flate_filter,
                          dynamic_compressed,
                          sizeof(dynamic_compressed) - 16)) {
        TEST_FAILURE();
    }

    unsigned char expected[1024];
    const size_t expected_len = build_dynamic_plain(expected, sizeof(expected));

    unsigned char output[1024] = {0};
    CosError error = {COS_ERROR_NONE, NULL};
    const size_t total = flate_read_all(fixture->flate_filter,
                                        output,
                                        sizeof(output),
                                        sizeof(output),
                                        &error);

    if (total >= expected_len) {
        TEST_FAILURE();
    }
    if (error.code != COS_ERROR_SYNTAX) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}

TEST_CASE_END

TEST_MAIN()
{
    TestFixture fixture = {0};

    TEST_RUN(decode_stored_block, &fixture);
    TEST_RUN(decode_fixed_huffman, &fixture);
    TEST_RUN(decode_dynamic_huffman, &fixture);
    TEST_RUN(decode_dynamic_small_reads, &fixture);
    TEST_RUN(decode_truncated_reports_error, &fixture);

    return EXIT_SUCCESS;
}
