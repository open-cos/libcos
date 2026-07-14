/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "CosXrefStreamParser.h"

#include "common/Assert.h"

#include <libcos/common/CosArray.h>
#include <libcos/common/CosData.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosString.h>
#include <libcos/objects/CosArrayObjNode.h>
#include <libcos/objects/CosDictObjNode.h>
#include <libcos/objects/CosIntObjNode.h>
#include <libcos/objects/CosNameObjNode.h>
#include <libcos/objects/CosObjNode.h>
#include <libcos/objects/CosStreamObjNode.h>
#include <libcos/xref/table/CosXrefEntry.h>
#include <libcos/xref/table/CosXrefSection.h>
#include <libcos/xref/table/CosXrefSubsection.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

static void
cos_xref_stream_entry_release_(void *item)
{
    COS_IMPL_PARAM_CHECK(item != NULL);
    if (COS_UNLIKELY(!item)) {
        return;
    }

    CosXrefEntry * const entry = *(CosXrefEntry **)item;
    free(entry);
}

static const CosArrayCallbacks cos_xref_stream_entry_array_callbacks_ = {
    .release = cos_xref_stream_entry_release_,
};

// Reads a big-endian unsigned integer of @p width bytes from @p bytes.
static uint64_t
cos_xref_stream_read_be_(const unsigned char *bytes,
                         unsigned int width)
{
    uint64_t value = 0;
    for (unsigned int i = 0; i < width; i++) {
        value = (value << 8) | (uint64_t)bytes[i];
    }
    return value;
}

// Reads an integer entry from the stream dictionary, or returns @p default_value when the entry
// is absent or not an integer.
static int
cos_xref_stream_dict_int_(CosDictObjNode *dict,
                          const char *key,
                          int default_value)
{
    CosObjNode *value = NULL;
    if (!cos_dict_obj_node_get_value_with_string(dict, key, &value, NULL) ||
        !value ||
        !cos_obj_node_is_integer(value)) {
        return default_value;
    }
    return cos_int_obj_node_get_value((CosIntObjNode *)value);
}

// Verifies the stream dictionary declares /Type /XRef.
static bool
cos_xref_stream_check_type_(CosDictObjNode *dict,
                            CosError * COS_Nullable out_error)
{
    CosObjNode *type_node = NULL;
    if (!cos_dict_obj_node_get_value_with_string(dict, "Type", &type_node, NULL) ||
        !type_node ||
        !cos_obj_node_is_name(type_node)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                           "Xref stream is missing a /Type /XRef entry"),
                            out_error);
        return false;
    }

    const CosString * const name = cos_name_obj_node_get_value((CosNameObjNode *)type_node);
    const char * const name_str = name ? cos_string_get_data(name) : NULL;
    if (!name_str || strcmp(name_str, "XRef") != 0) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                           "Xref stream /Type is not /XRef"),
                            out_error);
        return false;
    }

    return true;
}

// Reads the /W array of three non-negative field widths.
static bool
cos_xref_stream_read_w_(CosDictObjNode *dict,
                        unsigned int out_w[3],
                        CosError * COS_Nullable out_error)
{
    CosObjNode *w_node = NULL;
    if (!cos_dict_obj_node_get_value_with_string(dict, "W", &w_node, NULL) ||
        !w_node ||
        !cos_obj_node_is_array(w_node)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                           "Xref stream is missing a /W array"),
                            out_error);
        return false;
    }

    CosArrayObjNode * const w_array = (CosArrayObjNode *)w_node;
    if (cos_array_obj_node_get_count(w_array) < 3) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                           "Xref stream /W array must have three entries"),
                            out_error);
        return false;
    }

    for (size_t i = 0; i < 3; i++) {
        CosObjNode * const element = cos_array_obj_node_get_at(w_array, i, out_error);
        if (!element || !cos_obj_node_is_integer(element)) {
            COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                               "Xref stream /W entry is not an integer"),
                                out_error);
            return false;
        }
        const int width = cos_int_obj_node_get_value((CosIntObjNode *)element);
        if (width < 0) {
            COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                               "Xref stream /W entry is negative"),
                                out_error);
            return false;
        }
        out_w[i] = (unsigned int)width;
    }

    return true;
}

// Reads @p count entries from @p cursor into a new subsection and appends it to @p section,
// advancing @p cursor. The field values wider than 32 bits are truncated into the entry's
// unsigned int fields.
static bool
cos_xref_stream_add_subsection_(CosXrefSection *section,
                                const unsigned char **cursor,
                                const unsigned char *end,
                                unsigned int first_object_number,
                                unsigned int count,
                                const unsigned int w[3],
                                CosError * COS_Nullable out_error)
{
    const size_t entry_width = (size_t)w[0] + w[1] + w[2];

    CosArray *entries = cos_array_create(sizeof(CosXrefEntry *),
                                         &cos_xref_stream_entry_array_callbacks_,
                                         count);
    if (COS_UNLIKELY(!entries)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to allocate xref entries"),
                            out_error);
        return false;
    }

    CosXrefEntry *entry = NULL;
    CosXrefSubsection *subsection = NULL;

    for (unsigned int i = 0; i < count; i++) {
        if ((size_t)(end - *cursor) < entry_width) {
            COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                               "Xref stream data is shorter than /Index requires"),
                                out_error);
            goto failure;
        }

        entry = calloc(1, sizeof(CosXrefEntry));
        if (COS_UNLIKELY(!entry)) {
            COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                               "Failed to allocate xref entry"),
                                out_error);
            goto failure;
        }

        const unsigned char * const p = *cursor;
        // A zero-width type field defaults to type 1 (in-use).
        const uint64_t type = (w[0] == 0) ? 1u : cos_xref_stream_read_be_(p, w[0]);
        const uint64_t field1 = cos_xref_stream_read_be_(p + w[0], w[1]);
        const uint64_t field2 = cos_xref_stream_read_be_(p + w[0] + w[1], w[2]);
        *cursor += entry_width;

        switch (type) {
            case 1:
                cos_xref_entry_init_in_use(entry, (unsigned int)field1, (unsigned int)field2);
                break;
            case 2:
                cos_xref_entry_init_compressed(entry, (unsigned int)field1, (unsigned int)field2);
                break;
            case 0:
            default:
                // Type 0 is a free entry; any other type is treated as a null reference.
                cos_xref_entry_init_free(entry, (unsigned int)field1, (unsigned int)field2);
                break;
        }

        if (!cos_array_append_item(entries, &entry, out_error)) {
            goto failure;
        }
        entry = NULL; // Ownership copied into the array.
    }

    // cos_xref_subsection_create takes ownership of the entries array (and frees it on failure).
    subsection = cos_xref_subsection_create(first_object_number, count, entries);
    entries = NULL;
    if (COS_UNLIKELY(!subsection)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to allocate xref subsection"),
                            out_error);
        return false;
    }

    if (!cos_xref_section_add_subsection(section, subsection, out_error)) {
        cos_xref_subsection_destroy(subsection);
        return false;
    }

    return true;

failure:
    if (entry) {
        free(entry);
    }
    if (entries) {
        cos_array_destroy(entries);
    }
    return false;
}

CosXrefSection *
cos_xref_stream_parse_section_(CosStreamObjNode *xref_stream,
                               CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(xref_stream != NULL);

    CosDictObjNode * const dict = cos_stream_obj_node_get_dict(xref_stream);
    if (!dict) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                           "Xref stream has no dictionary"),
                            out_error);
        return NULL;
    }

    if (!cos_xref_stream_check_type_(dict, out_error)) {
        return NULL;
    }

    unsigned int w[3] = {0, 0, 0};
    if (!cos_xref_stream_read_w_(dict, w, out_error)) {
        return NULL;
    }
    if (((size_t)w[0] + w[1] + w[2]) == 0) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                           "Xref stream /W entries are all zero"),
                            out_error);
        return NULL;
    }

    CosData * const decoded = cos_stream_obj_node_get_decoded_data(xref_stream, out_error);
    if (!decoded) {
        return NULL;
    }

    CosXrefSection *section = cos_xref_section_create();
    if (COS_UNLIKELY(!section)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to allocate xref section"),
                            out_error);
        cos_data_free(decoded);
        return NULL;
    }

    const unsigned char *cursor = decoded->bytes;
    const unsigned char * const end = decoded->bytes + decoded->size;

    // Subsections come from /Index (pairs of first-object-number and count), defaulting to a
    // single subsection covering [0, /Size).
    CosObjNode *index_node = NULL;
    const bool have_index =
        cos_dict_obj_node_get_value_with_string(dict, "Index", &index_node, NULL) &&
        index_node != NULL &&
        cos_obj_node_is_array(index_node);

    if (have_index) {
        CosArrayObjNode * const index_array = (CosArrayObjNode *)index_node;
        const size_t index_count = cos_array_obj_node_get_count(index_array);
        if ((index_count % 2) != 0) {
            COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                               "Xref stream /Index must have an even number of entries"),
                                out_error);
            goto failure;
        }

        for (size_t i = 0; i < index_count; i += 2) {
            CosObjNode * const first_element = cos_array_obj_node_get_at(index_array, i, out_error);
            CosObjNode * const count_element = cos_array_obj_node_get_at(index_array, i + 1, out_error);
            if (!first_element || !count_element ||
                !cos_obj_node_is_integer(first_element) ||
                !cos_obj_node_is_integer(count_element)) {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                                   "Xref stream /Index entry is not an integer"),
                                    out_error);
                goto failure;
            }

            const int first = cos_int_obj_node_get_value((CosIntObjNode *)first_element);
            const int count = cos_int_obj_node_get_value((CosIntObjNode *)count_element);
            if (first < 0 || count < 0) {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                                   "Xref stream /Index entry is negative"),
                                    out_error);
                goto failure;
            }

            if (!cos_xref_stream_add_subsection_(section, &cursor, end, (unsigned int)first, (unsigned int)count, w, out_error)) {
                goto failure;
            }
        }
    }
    else {
        const int size = cos_xref_stream_dict_int_(dict, "Size", -1);
        if (size < 0) {
            COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                               "Xref stream is missing a valid /Size"),
                                out_error);
            goto failure;
        }

        if (!cos_xref_stream_add_subsection_(section, &cursor, end, 0, (unsigned int)size, w, out_error)) {
            goto failure;
        }
    }

    // The decoded data must be exactly the entries consumed; leftover bytes indicate a mismatch.
    if (cursor != end) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                           "Xref stream data does not match /W and /Index"),
                            out_error);
        goto failure;
    }

    cos_data_free(decoded);
    return section;

failure:
    cos_xref_section_destroy(section);
    cos_data_free(decoded);
    return NULL;
}

COS_ASSUME_NONNULL_END
