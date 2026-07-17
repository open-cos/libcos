/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/objects/CosObjStream.h"

#include "common/Assert.h"
#include "parse/CosObjParser.h"

#include <libcos/CosObjID.h>
#include <libcos/common/CosData.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosString.h>
#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>
#include <libcos/objects/CosDictObjNode.h>
#include <libcos/objects/CosIndirectObjNode.h>
#include <libcos/objects/CosIntObjNode.h>
#include <libcos/objects/CosNameObjNode.h>
#include <libcos/objects/CosObjNode.h>
#include <libcos/objects/CosReferenceObjNode.h>
#include <libcos/objects/CosStreamObjNode.h>
#include <libcos/syntax/tokenizer/CosTokenizer.h>

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

// The object number and (First-relative) byte offset of one contained object.
typedef struct CosObjStreamEntry {
    unsigned int obj_number;
    size_t offset;
} CosObjStreamEntry;

struct CosObjStream {
    CosDoc *doc; // Borrowed.

    CosData *decoded;      // Owned; the decoded object-stream contents.
    CosStream *mem_stream; // Owned; a read view over decoded->bytes.
    CosTokenizer *tokenizer;   // Owned.
    CosObjParser *obj_parser;  // Owned; borrows tokenizer.

    size_t count; // /N
    size_t first; // /First

    // /Extends: the object stream this one extends, or CosObjID_Invalid if there is none.
    CosObjID extends_id;

    CosObjStreamEntry *entries; // Owned; count elements.
};

// Reads a required non-negative integer entry from the object-stream dictionary.
static bool
cos_obj_stream_dict_int_(CosDictObjNode *dict,
                         const char *key,
                         size_t *out_value,
                         CosError * COS_Nullable out_error)
{
    CosObjNode *value = NULL;
    if (!cos_dict_obj_node_get_value_with_string(dict, key, &value, NULL) ||
        !value ||
        !cos_obj_node_is_integer(value)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_PARSE,
                                           "Object stream dictionary is missing a required integer entry"),
                            out_error);
        return false;
    }

    const int int_value = cos_int_obj_node_get_value((CosIntObjNode *)value);
    if (int_value < 0) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_PARSE,
                                           "Object stream dictionary entry is negative"),
                            out_error);
        return false;
    }

    *out_value = (size_t)int_value;
    return true;
}

// Verifies the stream dictionary declares /Type /ObjStm.
static bool
cos_obj_stream_check_dict_(CosDictObjNode *dict,
                           CosError * COS_Nullable out_error)
{
    CosObjNode *type_node = NULL;
    if (!cos_dict_obj_node_get_value_with_string(dict, "Type", &type_node, NULL) ||
        !type_node ||
        !cos_obj_node_is_name(type_node)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_PARSE,
                                           "Object stream is missing a /Type /ObjStm entry"),
                            out_error);
        return false;
    }

    const CosString * const name = cos_name_obj_node_get_value((CosNameObjNode *)type_node);
    const char * const name_str = name ? cos_string_get_data(name) : NULL;
    if (!name_str || strcmp(name_str, "ObjStm") != 0) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_PARSE,
                                           "Object stream /Type is not /ObjStm"),
                            out_error);
        return false;
    }

    return true;
}

// Reads the optional /Extends entry: the reference to the object stream that this one extends.
// It is a hint only -- the chain is not followed -- so a missing or malformed entry is not an
// error and yields CosObjID_Invalid.
static CosObjID
cos_obj_stream_read_extends_(CosDictObjNode *dict)
{
    CosObjNode *extends_node = NULL;
    if (!cos_dict_obj_node_get_value_with_string(dict, "Extends", &extends_node, NULL) ||
        !extends_node ||
        cos_obj_node_get_type(extends_node) != CosObjNodeType_Reference) {
        return CosObjID_Invalid;
    }

    return cos_reference_obj_node_get_id((CosReferenceObjNode *)extends_node);
}

// Parses the @p count header pairs (object number and First-relative offset) into @p entries.
// Operates on the freshly built parser/stream before the object-stream struct is allocated.
static bool
cos_obj_stream_parse_header_(CosStream *mem_stream,
                             CosTokenizer *tokenizer,
                             CosObjParser *obj_parser,
                             CosObjStreamEntry *entries,
                             size_t count,
                             size_t available,
                             CosError * COS_Nullable out_error)
{
    if (count == 0) {
        return true;
    }

    if (!cos_stream_seek(mem_stream, 0, CosStreamOffsetWhence_Set, out_error)) {
        return false;
    }
    cos_tokenizer_reset(tokenizer);
    cos_obj_parser_flush_tokens_(obj_parser);

    for (size_t i = 0; i < count; i++) {
        int values[2] = {0, 0};
        for (size_t j = 0; j < 2; j++) {
            CosObjNode * const node = cos_obj_parser_next_object(obj_parser, out_error);
            if (!node) {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_PARSE,
                                                   "Object stream header ended prematurely"),
                                    out_error);
                return false;
            }
            const bool is_integer = cos_obj_node_is_integer(node);
            if (is_integer) {
                values[j] = cos_int_obj_node_get_value((CosIntObjNode *)node);
            }
            cos_obj_node_release(node);
            if (!is_integer) {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_PARSE,
                                                   "Object stream header entry is not an integer"),
                                    out_error);
                return false;
            }
        }

        if (values[0] < 0 || values[1] < 0 || (size_t)values[1] > available) {
            COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_PARSE,
                                               "Object stream header has an out-of-range entry"),
                                out_error);
            return false;
        }

        entries[i].obj_number = (unsigned int)values[0];
        entries[i].offset = (size_t)values[1];
    }

    return true;
}

CosObjStream *
cos_obj_stream_create(const CosStreamObjNode *stream_obj,
                      CosDoc *doc,
                      CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(stream_obj != NULL);
    COS_API_PARAM_CHECK(doc != NULL);
    if (COS_UNLIKELY(!stream_obj || !doc)) {
        return NULL;
    }

    CosDictObjNode * const dict = cos_stream_obj_node_get_dict(stream_obj);
    if (!dict) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_PARSE,
                                           "Object stream has no dictionary"),
                            out_error);
        return NULL;
    }

    if (!cos_obj_stream_check_dict_(dict, out_error)) {
        return NULL;
    }

    size_t count = 0;
    size_t first = 0;
    if (!cos_obj_stream_dict_int_(dict, "N", &count, out_error) ||
        !cos_obj_stream_dict_int_(dict, "First", &first, out_error)) {
        return NULL;
    }

    const CosObjID extends_id = cos_obj_stream_read_extends_(dict);

    CosObjStream *obj_stream = NULL;
    CosData *decoded = NULL;
    CosStream *mem_stream = NULL;
    CosTokenizer *tokenizer = NULL;
    CosObjParser *obj_parser = NULL;
    CosObjStreamEntry *entries = NULL;

    decoded = cos_stream_obj_node_get_decoded_data(stream_obj, out_error);
    if (!decoded) {
        return NULL;
    }

    if (first > decoded->size) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_PARSE,
                                           "Object stream /First is beyond the decoded data"),
                            out_error);
        goto failure;
    }

    mem_stream = (CosStream *)cos_memory_stream_create_readonly(decoded->bytes, decoded->size);
    if (!mem_stream) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to create object stream buffer"),
                            out_error);
        goto failure;
    }

    tokenizer = cos_tokenizer_create(mem_stream);
    if (!tokenizer) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to create object stream tokenizer"),
                            out_error);
        goto failure;
    }

    obj_parser = cos_obj_parser_create_with_tokenizer(doc, tokenizer);
    if (!obj_parser) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to create object stream parser"),
                            out_error);
        goto failure;
    }

    // An object stream holds bare direct objects: the integer header pairs and
    // the compressed members. Never an indirect definition or reference.
    cos_obj_parser_set_top_level_flags_(obj_parser,
                                        CosObjParserFlag_DirectObj);

    if (count > 0) {
        entries = calloc(count, sizeof(CosObjStreamEntry));
        if (!entries) {
            COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                               "Failed to allocate object stream entries"),
                                out_error);
            goto failure;
        }
    }

    // Parse the header before allocating the struct, so failures unwind through the locals here.
    if (!cos_obj_stream_parse_header_(mem_stream,
                                      tokenizer,
                                      obj_parser,
                                      entries,
                                      count,
                                      decoded->size - first,
                                      out_error)) {
        goto failure;
    }

    obj_stream = calloc(1, sizeof(CosObjStream));
    if (!obj_stream) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to allocate object stream"),
                            out_error);
        goto failure;
    }

    obj_stream->doc = doc;
    obj_stream->decoded = decoded;
    obj_stream->mem_stream = mem_stream;
    obj_stream->tokenizer = tokenizer;
    obj_stream->obj_parser = obj_parser;
    obj_stream->count = count;
    obj_stream->first = first;
    obj_stream->extends_id = extends_id;
    obj_stream->entries = entries;

    return obj_stream;

failure:
    if (obj_parser) {
        cos_obj_parser_destroy(obj_parser);
    }
    if (tokenizer) {
        cos_tokenizer_destroy(tokenizer);
    }
    if (mem_stream) {
        cos_stream_close(mem_stream);
    }
    if (entries) {
        free(entries);
    }
    if (decoded) {
        cos_data_free(decoded);
    }
    return NULL;
}

void
cos_obj_stream_destroy(CosObjStream *obj_stream)
{
    if (!obj_stream) {
        return;
    }

    // The obj parser borrows the tokenizer, which borrows the memory stream, which reads from
    // the decoded buffer; tear down in that dependency order.
    if (obj_stream->obj_parser) {
        cos_obj_parser_destroy(COS_nonnull_cast(obj_stream->obj_parser));
    }
    if (obj_stream->tokenizer) {
        cos_tokenizer_destroy(COS_nonnull_cast(obj_stream->tokenizer));
    }
    if (obj_stream->mem_stream) {
        cos_stream_close(COS_nonnull_cast(obj_stream->mem_stream));
    }
    if (obj_stream->entries) {
        free(obj_stream->entries);
    }
    if (obj_stream->decoded) {
        cos_data_free(COS_nonnull_cast(obj_stream->decoded));
    }
    free(obj_stream);
}

size_t
cos_obj_stream_get_count(const CosObjStream *obj_stream)
{
    COS_API_PARAM_CHECK(obj_stream != NULL);
    if (COS_UNLIKELY(!obj_stream)) {
        return 0;
    }

    return obj_stream->count;
}

CosObjID
cos_obj_stream_get_extends(const CosObjStream *obj_stream)
{
    COS_API_PARAM_CHECK(obj_stream != NULL);
    if (COS_UNLIKELY(!obj_stream)) {
        return CosObjID_Invalid;
    }

    return obj_stream->extends_id;
}

bool
cos_obj_stream_get_obj_number_at(const CosObjStream *obj_stream,
                                 size_t index,
                                 unsigned int *out_obj_number,
                                 CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(obj_stream != NULL);
    COS_API_PARAM_CHECK(out_obj_number != NULL);
    if (COS_UNLIKELY(!obj_stream || !out_obj_number)) {
        return false;
    }

    if (index >= obj_stream->count) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Object stream index is out of range"),
                            out_error);
        return false;
    }

    *out_obj_number = obj_stream->entries[index].obj_number;
    return true;
}

CosObjNode *
cos_obj_stream_get_object_at(CosObjStream *obj_stream,
                             size_t index,
                             CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(obj_stream != NULL);
    if (COS_UNLIKELY(!obj_stream)) {
        return NULL;
    }

    if (index >= obj_stream->count) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Object stream index is out of range"),
                            out_error);
        return NULL;
    }

    const CosStreamOffset position =
        (CosStreamOffset)obj_stream->first + (CosStreamOffset)obj_stream->entries[index].offset;

    if (!cos_stream_seek(obj_stream->mem_stream, position, CosStreamOffsetWhence_Set, out_error)) {
        return NULL;
    }
    cos_tokenizer_reset(obj_stream->tokenizer);
    cos_obj_parser_flush_tokens_(obj_stream->obj_parser);

    // Objects in an object stream are stored bare (no obj/endobj). Wrap the parsed value in an
    // indirect node carrying its object number, so callers see the same shape as an object
    // loaded directly from the file. The generation number of a compressed object is always 0.
    CosObjNode * const direct = cos_obj_parser_next_object(obj_stream->obj_parser, out_error);
    if (!direct) {
        return NULL;
    }

    const CosObjID id = cos_obj_id_make(obj_stream->entries[index].obj_number, 0);
    CosIndirectObjNode * const indirect = cos_indirect_obj_node_alloc(id, direct);
    if (!indirect) {
        cos_obj_node_release(direct);
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to wrap object stream object"),
                            out_error);
        return NULL;
    }

    return (CosObjNode *)indirect;
}

COS_ASSUME_NONNULL_END
