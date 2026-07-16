/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_TESTS_FUZZ_COS_FUZZ_H
#define LIBCOS_TESTS_FUZZ_COS_FUZZ_H

#include <stddef.h>

/**
 * Inputs larger than this are rejected by every target.
 *
 * Beyond this size the fuzzer spends its time on throughput rather than on
 * reaching new code, and outsized inputs trip the libFuzzer timeout without
 * indicating a real defect.
 */
#define COS_FUZZ_MAX_INPUT_SIZE ((size_t)(1024 * 1024))

#endif /* LIBCOS_TESTS_FUZZ_COS_FUZZ_H */
