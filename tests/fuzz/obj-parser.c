/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "CosFuzz.h"

#include "parse/CosObjParser.h"

#include <libcos/CosDoc.h>
#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>
#include <libcos/objects/CosObjNode.h>

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

    CosDoc * const doc = cos_doc_create(NULL);
    if (!doc) {
        cos_stream_close((CosStream *)stream);
        return 0;
    }

    CosObjParser * const parser = cos_obj_parser_create(doc, (CosStream *)stream);
    if (!parser) {
        cos_doc_destroy(doc);
        cos_stream_close((CosStream *)stream);
        return 0;
    }

    while (cos_obj_parser_has_next_object(parser)) {
        CosObjNode * const obj = cos_obj_parser_next_object(parser, NULL);
        if (!obj) {
            break;
        }

        cos_obj_node_release(obj);
    }

    cos_obj_parser_destroy(parser);
    cos_doc_destroy(doc);
    cos_stream_close((CosStream *)stream);

    return 0;
}
