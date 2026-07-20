/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "CosFuzz.h"

#include <libcos/CosDoc.h>
#include <libcos/CosParser.h>
#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>

#include <stddef.h>
#include <stdint.h>

extern int
LLVMFuzzerTestOneInput(const uint8_t *data,
                       size_t size);

extern int
LLVMFuzzerTestOneInput(const uint8_t *data,
                       size_t size)
{
    if (size > COS_FUZZ_MAX_INPUT_SIZE) {
        return 0;
    }

    CosMemoryStream * const stream = cos_memory_stream_create_readonly(data, size);
    if (!stream) {
        return 0;
    }

    CosDoc * const doc = cos_doc_create();
    if (!doc) {
        cos_stream_close((CosStream *)stream);
        return 0;
    }

    /* The document owns the parser; the parser only borrows the stream. */
    CosParser * const parser = cos_parser_create(doc, (CosStream *)stream, NULL);
    if (parser) {
        (void)cos_parser_parse(parser, NULL);
    }

    cos_doc_destroy(doc);
    cos_stream_close((CosStream *)stream);

    return 0;
}
