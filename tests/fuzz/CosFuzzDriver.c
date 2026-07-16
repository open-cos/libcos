/*
 * Copyright (c) 2025 OpenCOS.
 */

/*
 * A standalone driver for the fuzz targets.
 *
 * libFuzzer supplies its own main(), so this file is only compiled when
 * COS_BUILD_FOR_FUZZING is OFF. It lets the harnesses build and replay a
 * corpus with any compiler, without sanitizers or libFuzzer.
 *
 * Each argument is a file whose contents are passed to the target once.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern int
LLVMFuzzerTestOneInput(const uint8_t *data,
                       size_t size);

static int
run_file(const char *path)
{
    FILE * const file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "cos-fuzz: cannot open %s\n", path);
        return EXIT_FAILURE;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "cos-fuzz: cannot seek %s\n", path);
        fclose(file);
        return EXIT_FAILURE;
    }

    const long length = ftell(file);
    if (length < 0) {
        fprintf(stderr, "cos-fuzz: cannot size %s\n", path);
        fclose(file);
        return EXIT_FAILURE;
    }
    rewind(file);

    const size_t size = (size_t)length;

    /* malloc(0) may return NULL, which is not a failure here. */
    uint8_t * const data = (size > 0) ? malloc(size) : NULL;
    if (size > 0 && !data) {
        fprintf(stderr, "cos-fuzz: out of memory for %s\n", path);
        fclose(file);
        return EXIT_FAILURE;
    }

    const size_t read_count = (size > 0) ? fread(data, 1, size, file) : 0;
    fclose(file);

    if (read_count != size) {
        fprintf(stderr, "cos-fuzz: short read on %s\n", path);
        free(data);
        return EXIT_FAILURE;
    }

    /* An empty input is a legitimate test case; the target must tolerate it. */
    (void)LLVMFuzzerTestOneInput(data ? data : (const uint8_t *)"", size);

    free(data);

    return EXIT_SUCCESS;
}

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s FILE...\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; i++) {
        if (run_file(argv[i]) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
        printf("cos-fuzz: ok %s\n", argv[i]);
    }

    return EXIT_SUCCESS;
}
