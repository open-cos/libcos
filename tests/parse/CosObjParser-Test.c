/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/CosDoc.h"
#include "libcos/io/CosFileInputStream.h"
#include "libcos/objects/CosObj.h"
#include "parse/CosObjParser.h"
#include "syntax/CosSyntax.h"

#include <libcos/io/CosInputStream.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

int
main(int argc, char * COS_Nonnull argv[])
{
    // Get the current working directory.

    CosFileInputStream * const input_stream = cos_file_input_stream_open("/home/david/Projects/C/libcos/tests/data/Hello-world.pdf",
                                                                         "r",
                                                                         NULL);
    if (!input_stream) {
        goto failure;
    }

    CosDoc * const doc = cos_doc_alloc();

    CosObjParser * const parser = cos_obj_parser_alloc(doc,
                                                       (CosInputStream *)input_stream);
    if (!parser) {
        goto failure;
    }

    while (cos_obj_parser_has_next_object(parser)) {
        CosObj * const obj = cos_obj_parser_next_object(parser,
                                                        NULL);
        if (!obj) {
            goto failure;
        }

        cos_obj_print_description(obj);
    }

    cos_obj_parser_free(parser);
    cos_doc_free(doc);
    cos_input_stream_close((CosInputStream *)input_stream);

    return EXIT_SUCCESS;

failure:
    return EXIT_FAILURE;
}

COS_ASSUME_NONNULL_END
