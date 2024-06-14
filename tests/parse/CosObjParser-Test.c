/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "parse/CosObjParser.h"

#include <libcos/CosDoc.h>
#include <libcos/io/CosFileStream.h>
#include <libcos/objects/CosObj.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

int
main(COS_ATTR_UNUSED int argc,
     COS_ATTR_UNUSED char * COS_Nonnull argv[])
{
    // Get the current working directory.

    CosStream * const input_stream = cos_file_stream_create("/home/david/Projects/C/libcos/tests/data/Hello-world.pdf",
                                                            "r");
    if (!input_stream) {
        goto failure;
    }

    CosDoc * const doc = cos_doc_create(NULL);
    if (!doc) {
        goto failure;
    }

    CosObjParser * const parser = cos_obj_parser_create(doc,
                                                        input_stream);
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
    cos_stream_close(input_stream);

    return EXIT_SUCCESS;

failure:
    if (input_stream) {
        cos_stream_close(input_stream);
    }
    return EXIT_FAILURE;
}

COS_ASSUME_NONNULL_END
