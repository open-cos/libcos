/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "CosFuzz.h"

#include "filters/CosFilterFactory.h"

#include <libcos/common/CosString.h>
#include <libcos/filters/CosFilter.h>
#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>

#include <stddef.h>
#include <stdint.h>

/* Every decode filter the factory supports, keyed by the input's first byte. */
static const char * const filter_names_[] = {
    "FlateDecode",
    "ASCIIHexDecode",
    "ASCII85Decode",
    "RunLengthDecode",
};

#define FILTER_NAME_COUNT (sizeof(filter_names_) / sizeof(filter_names_[0]))

extern int
LLVMFuzzerTestOneInput(const uint8_t *data,
                       size_t size);

extern int
LLVMFuzzerTestOneInput(const uint8_t *data,
                       size_t size)
{
    /* The first byte selects the filter; the rest is the encoded data. */
    if (size < 1 || size > COS_FUZZ_MAX_INPUT_SIZE) {
        return 0;
    }

    CosString * const name = cos_string_alloc_with_str(filter_names_[data[0] % FILTER_NAME_COUNT]);
    if (!name) {
        return 0;
    }

    /* Vary the end-of-data strictness across Off/Warn/Error so the fuzzer
     * covers the missing/malformed-marker reporting paths. */
    const CosFilterOptions filter_options = {
        .eod_strict_level = (CosStrictLevel)(data[0] % 3),
        .diagnostic_handler = NULL,
    };
    CosStream * const filter = cos_filter_create_for_name_(name, &filter_options, NULL);
    cos_string_free(name);
    if (!filter) {
        return 0;
    }

    CosMemoryStream * const source = cos_memory_stream_create_readonly(data + 1, size - 1);
    if (!source) {
        cos_stream_close(filter);
        return 0;
    }

    /* The filter takes ownership of the source; closing it closes the source. */
    cos_filter_attach_source((CosFilter *)filter, (CosStream *)source);

    unsigned char buffer[512];
    while (cos_stream_read(filter, buffer, sizeof(buffer), NULL) > 0) {
        /* Drain the decoded output. */
    }

    cos_stream_close(filter);

    return 0;
}
