/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "common/Assert.h"

#include <libcos/common/CosDefines.h>
#include <libcos/filters/CosASCII85Filter.h>
#include <libcos/io/CosMemoryStream.h>

#include <stdio.h>
#include <stdlib.h>

typedef struct ascii85_sample {
    char *input;
    size_t input_size;
    char *expected_output;
    size_t expected_output_size;
} ascii85_sample;

extern int
TEST_NAME(int argc,
          char *argv[]);

int
TEST_NAME(COS_ATTR_UNUSED int argc,
          COS_ATTR_UNUSED char *argv[])
{
    // Encoded input and decoded output.
    const ascii85_sample samples[] = {
        {
            "@:E_WAS,RgBkhF\"D/O92EH6,BF`qtRH$T~>",
            36,
            "abcdefghijklmnopqrstuvwxyz",
            26,
        },
    };

    for (size_t i = 0; i < sizeof(samples) / sizeof(samples[0]); i++) {
        const ascii85_sample sample = samples[i];

        CosMemoryStream * const input_stream = cos_memory_stream_create(sample.input,
                                                                        sample.input_size,
                                                                        false);

        CosASCII85Filter * const ascii85_filter = cos_ascii85_filter_create();
        cos_filter_attach_source((CosFilter *)ascii85_filter,
                                 (CosStream *)input_stream);

        char output[256] = {0};
        size_t output_size = cos_stream_read((CosStream *)ascii85_filter,
                                             output,
                                             sizeof(output) - 1,
                                             NULL);
        if (output_size != sample.expected_output_size) {
            (void)fprintf(stderr,
                          "Output size mismatch: %zu != %zu\n",
                          output_size,
                          sample.expected_output_size);
            return EXIT_FAILURE;
        }

        cos_stream_close((CosStream *)ascii85_filter);
    }

    return 0;
}
