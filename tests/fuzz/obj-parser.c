/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "CosFuzz.h"

#include "parse/CosObjParser.h"

#include <libcos/CosDoc.h>
#include <libcos/common/CosData.h>
#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>
#include <libcos/objects/CosIndirectObjNode.h>
#include <libcos/objects/CosObjNode.h>
#include <libcos/objects/CosStreamObjNode.h>

#include <stddef.h>
#include <stdint.h>

extern int
LLVMFuzzerTestOneInput(const uint8_t *data,
                       size_t size);

/**
 * Decodes a stream object's contents, when the node is one.
 *
 * Parsing a stream object only records where its data begins and ends; the
 * filter chain -- and with it every decoder and the predictor -- runs on the
 * first read. Without this, the whole of src/filters/ is unreachable from this
 * target no matter what the input contains.
 *
 * The decoded output is drained in fixed-size chunks rather than fetched in one
 * buffer, because a valid FlateDecode stream can expand without bound and this
 * target's inputs are attacker-controlled: cos_stream_obj_node_get_decoded_data
 * would happily allocate the whole expansion.
 */
static void
cos_fuzz_decode_stream_(CosObjNode *obj)
{
    CosObjNode *value = obj;

    /* An indirect object definition wraps the stream. */
    if (cos_obj_node_get_type(obj) == CosObjNodeType_Indirect) {
        value = cos_indirect_obj_node_get_value((CosIndirectObjNode *)obj);
    }

    if (!value || !cos_obj_node_is_stream(value)) {
        return;
    }

    CosStreamObjNode * const stream_obj = (CosStreamObjNode *)value;

    /*
     * The accessors are part of the attack surface too: each one reads the
     * stream dictionary, so each one is reachable with an attacker-controlled
     * /Length, /Filter or /DecodeParms.
     */
    (void)cos_stream_obj_node_get_dict(stream_obj);
    (void)cos_stream_obj_node_get_data(stream_obj);
    (void)cos_stream_obj_node_get_length(stream_obj);
    (void)cos_stream_obj_node_get_filter_names(stream_obj, NULL);

    size_t length_hint = 0;
    if (cos_stream_obj_node_get_decoded_length_hint(stream_obj, &length_hint, NULL) &&
        length_hint <= COS_FUZZ_MAX_INPUT_SIZE) {
        /*
         * Only take the one-shot path when the stream itself claims a modest
         * size. The hint comes from the input, so it is not to be trusted as a
         * bound -- it is used here purely to keep the common case cheap, with
         * the chunked drain below as the real limit.
         */
        CosData * const data = cos_stream_obj_node_get_decoded_data(stream_obj, NULL);
        if (data) {
            cos_data_free(data);
        }
    }

    CosStream * const decoded =
        cos_stream_obj_node_create_decode_stream(stream_obj, NULL);
    if (!decoded) {
        return;
    }

    unsigned char buffer[512];
    size_t total = 0;

    for (;;) {
        const size_t bytes_read = cos_stream_read(decoded,
                                                  buffer,
                                                  sizeof(buffer),
                                                  NULL);
        if (bytes_read == 0) {
            break;
        }

        total += bytes_read;
        if (total >= COS_FUZZ_MAX_INPUT_SIZE) {
            break;
        }
    }

    cos_stream_close(decoded);
}

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

        cos_fuzz_decode_stream_(obj);

        cos_obj_node_release(obj);
    }

    cos_obj_parser_destroy(parser);
    cos_doc_destroy(doc);
    cos_stream_close((CosStream *)stream);

    return 0;
}
