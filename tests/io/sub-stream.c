/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "CosTest.h"

#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>
#include <libcos/io/CosSubStream.h>

#include <string.h>

static const char g_source_bytes[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
#define SOURCE_SIZE (sizeof(g_source_bytes) - 1)

typedef struct TestFixture {
    CosStream *source;
} TestFixture;

static bool
setup(TestFixture *fixture)
{
    CosMemoryStream * const source =
        cos_memory_stream_create_readonly(g_source_bytes, SOURCE_SIZE);
    if (COS_UNLIKELY(!source)) {
        return false;
    }

    fixture->source = (CosStream *)source;
    return true;
}

static void
teardown(TestFixture *fixture)
{
    if (fixture->source) {
        cos_stream_close(fixture->source);
        fixture->source = NULL;
    }
}

TEST_CASE_BEGIN(read_full_window)
{
    /* Window "FGHIJ" at offset 5. */
    CosStream * const window = cos_sub_stream_create(fixture->source, 5, 5, false, NULL);
    if (!window) {
        TEST_FAILURE();
    }

    char output[16] = {0};
    const size_t read_count = cos_stream_read(window, output, sizeof(output), NULL);

    cos_stream_close(window);

    if (read_count != 5 || memcmp(output, "FGHIJ", 5) != 0) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}

TEST_CASE_END

TEST_CASE_BEGIN(read_clamps_at_tail)
{
    /* Window of 4 bytes at the very end ("WXYZ"); a larger read is clamped. */
    CosStream * const window = cos_sub_stream_create(fixture->source, SOURCE_SIZE - 4, 4, false, NULL);
    if (!window) {
        TEST_FAILURE();
    }

    char output[16] = {0};
    const size_t read_count = cos_stream_read(window, output, sizeof(output), NULL);
    const size_t tail_count = cos_stream_read(window, output, sizeof(output), NULL);

    cos_stream_close(window);

    if (read_count != 4 || memcmp(output, "WXYZ", 4) != 0 || tail_count != 0) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}

TEST_CASE_END

TEST_CASE_BEGIN(seek_within_window)
{
    /* Full-source window; seek forward then read. */
    CosStream * const window = cos_sub_stream_create(fixture->source, 0, SOURCE_SIZE, false, NULL);
    if (!window) {
        TEST_FAILURE();
    }

    if (!cos_stream_seek(window, 10, CosStreamOffsetWhence_Set, NULL)) {
        cos_stream_close(window);
        TEST_FAILURE();
    }
    if (cos_stream_get_position(window, NULL) != 10) {
        cos_stream_close(window);
        TEST_FAILURE();
    }

    char output[4] = {0};
    const size_t read_count = cos_stream_read(window, output, 3, NULL);

    /* Seek relative to end (-1) lands on the last byte 'Z'. */
    char last = 0;
    const bool sought_end = cos_stream_seek(window, -1, CosStreamOffsetWhence_End, NULL);
    const size_t last_count = cos_stream_read(window, &last, 1, NULL);

    cos_stream_close(window);

    if (read_count != 3 || memcmp(output, "KLM", 3) != 0 ||
        !sought_end || last_count != 1 || last != 'Z') {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}

TEST_CASE_END

TEST_CASE_BEGIN(eof_at_length)
{
    CosStream * const window = cos_sub_stream_create(fixture->source, 5, 5, false, NULL);
    if (!window) {
        TEST_FAILURE();
    }

    char output[8] = {0};
    (void)cos_stream_read(window, output, sizeof(output), NULL);

    const bool at_end = cos_stream_is_at_end(window, NULL);

    cos_stream_close(window);

    if (!at_end) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}

TEST_CASE_END

TEST_CASE_BEGIN(disjoint_windows_share_source)
{
    /* Two windows over the same source, read interleaved, exercising seek-before-read. */
    CosStream * const window_a = cos_sub_stream_create(fixture->source, 0, 5, false, NULL);
    CosStream * const window_b = cos_sub_stream_create(fixture->source, 10, 5, false, NULL);
    if (!window_a || !window_b) {
        if (window_a) {
            cos_stream_close(window_a);
        }
        if (window_b) {
            cos_stream_close(window_b);
        }
        TEST_FAILURE();
    }

    char a[8] = {0};
    char b[8] = {0};
    size_t a_len = 0;
    size_t b_len = 0;

    a_len += cos_stream_read(window_a, a + a_len, 2, NULL); /* "AB" */
    b_len += cos_stream_read(window_b, b + b_len, 3, NULL); /* "KLM" */
    a_len += cos_stream_read(window_a, a + a_len, 8, NULL); /* "CDE" */
    b_len += cos_stream_read(window_b, b + b_len, 8, NULL); /* "NO" */

    cos_stream_close(window_a);
    cos_stream_close(window_b);

    if (a_len != 5 || memcmp(a, "ABCDE", 5) != 0 ||
        b_len != 5 || memcmp(b, "KLMNO", 5) != 0) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}

TEST_CASE_END

TEST_MAIN()
{
    TestFixture fixture = {0};

    TEST_RUN(read_full_window, &fixture);
    TEST_RUN(read_clamps_at_tail, &fixture);
    TEST_RUN(seek_within_window, &fixture);
    TEST_RUN(eof_at_length, &fixture);
    TEST_RUN(disjoint_windows_share_source, &fixture);

    return EXIT_SUCCESS;
}
