/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "../../src/parse/CosObjParser.h"
#include "objects/CosObj.h"

#include <libcos/CosDoc.h>

#include <stddef.h>
#include <stdint.h>

extern int
LLVMFuzzerTestOneInput(const uint8_t *data,
                       size_t size);

extern int
LLVMFuzzerTestOneInput(const uint8_t *data,
                       size_t size)
{
    CosInputStream *stream = cos_memory_input_stream_open((unsigned char *)data, size);

    CosDoc *doc = cos_doc_create(NULL);

    CosObjParser *parser = cos_obj_parser_create(doc,
                                                stream);

    if (!parser) {
        goto failure;
    }

    while (cos_obj_parser_has_next_object(parser)) {
        CosObj * const obj = cos_obj_parser_next_object(parser,
                                                        NULL);
        if (!obj) {
            goto failure;
        }

        cos_obj_print_desc(obj);

        cos_obj_free(obj);
    }

    cos_obj_parser_destroy(parser);
    cos_doc_destroy(doc);
    cos_input_stream_close(stream);

    return 0;

failure:
    cos_obj_parser_destroy(parser);
    cos_doc_destroy(doc);
    cos_input_stream_close(stream);
    return 0;
}
