/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_COS_TEST_H
#define LIBCOS_COS_TEST_H

#include <stdlib.h>

#define TEST_MAIN()                     \
    extern int                          \
    TEST_NAME(int argc,                 \
              char *argv[]);            \
                                        \
    int                                 \
    TEST_NAME(COS_ATTR_UNUSED int argc, \
              COS_ATTR_UNUSED char *argv[])

#define TEST_PROLOGUE \
    int test_result = EXIT_SUCCESS;

#define TEST_EPILOGUE \
test_epilogue:        \
    return test_result;

// clang-format off

#define TEST_CASE_BEGIN(name)             \
static int name(TestFixture *fixture) \
{                                     \
    TEST_PROLOGUE

#define TEST_CASE_END \
    TEST_EPILOGUE \
}

// clang-format on

#define TEST_RUN(test, fixture)       \
    do {                              \
        if (!setup(fixture)) {        \
            return EXIT_FAILURE;      \
        }                             \
        int result = test(fixture);   \
        teardown(fixture);            \
        if (result != EXIT_SUCCESS) { \
            return result;            \
        }                             \
    } while (0)

#define TEST_SUCCESS()          \
    test_result = EXIT_SUCCESS; \
    goto test_epilogue;         \
    (void)(0)

#define TEST_FAILURE()          \
    test_result = EXIT_FAILURE; \
    goto test_epilogue;         \
    (void)(0)

#endif /* LIBCOS_COS_TEST_H */
