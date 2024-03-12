/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/CosDoc.h"
#include "libcos/io/CosFileInputStream.h"
#include "libcos/objects/CosObj.h"
#include "libcos/syntax/CosKeywords.h"
#include "parse/CosObjParser.h"

#include <libcos/io/CosInputStream.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

int
main(COS_ATTR_UNUSED int argc,
     COS_ATTR_UNUSED char * COS_Nonnull argv[])
{
    // Get the current working directory.

    CosInputStream * const input_stream = (CosInputStream * const)cos_file_input_stream_open("/home/david/Projects/C/libcos/tests/data/Hello-world.pdf",
                                                                                             "r",
                                                                                             NULL);
    if (!input_stream) {
        goto failure;
    }

    CosDoc * const doc = cos_doc_create(NULL);

    CosObjParser * const parser = cos_obj_parser_alloc(doc,
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

    cos_obj_parser_free(parser);
    cos_doc_destroy(doc);
    cos_input_stream_close(input_stream);

    return EXIT_SUCCESS;

failure:
    return EXIT_FAILURE;
}

COS_ASSUME_NONNULL_END
