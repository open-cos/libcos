/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "CosTest.h"

#include <libcos/common/CosData.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosString.h>
#include <libcos/objects/CosArrayObjNode.h>
#include <libcos/objects/CosDictObjNode.h>
#include <libcos/objects/CosIntObjNode.h>
#include <libcos/objects/CosNameObjNode.h>
#include <libcos/objects/CosStreamObjNode.h>

#include <string.h>

// Test vectors produced with Python (zlib, base64.a85encode, binascii.hexlify) and
// verified against their decoders before embedding.

// clang-format off

// FlateDecode of the shared plain text below.
static const unsigned char flate_encoded[] = {
    0x78, 0xda, 0x0b, 0x2e, 0x29, 0x4a, 0x4d, 0xcc, 0x2d, 0x56, 0x48, 0x49,
    0x4d, 0xce, 0x4f, 0x49, 0x55, 0x28, 0xc9, 0x28, 0xca, 0x2f, 0x4d, 0xcf,
    0x00, 0xd2, 0xa9, 0x0a, 0x69, 0x99, 0x39, 0x25, 0xa9, 0x45, 0x0a, 0x05,
    0x99, 0x05, 0xa9, 0x39, 0x99, 0x79, 0xa9, 0x7a, 0x0a, 0xc1, 0x34, 0x52,
    0x0b, 0x00, 0xcd, 0x38, 0x30, 0xee,
};

// ASCIIHexDecode ("Hello, Hex!" as uppercase hex with a '>' terminator).
static const unsigned char ascii_hex_encoded[] = {
    0x34, 0x38, 0x36, 0x35, 0x36, 0x43, 0x36, 0x43, 0x36, 0x46, 0x32, 0x43,
    0x32, 0x30, 0x34, 0x38, 0x36, 0x35, 0x37, 0x38, 0x32, 0x31, 0x3e,
};
static const char ascii_hex_plain[] = "Hello, Hex!";

// Cascade: ascii85(flate(plain)) with the '~>' terminator, decoded by the
// filter chain [ASCII85Decode FlateDecode] back to the shared plain text.
static const unsigned char cascade_encoded[] = {
    0x47, 0x68, 0x4e, 0x4e, 0x58, 0x2e, 0x38, 0x21, 0x3d, 0x59, 0x2f, 0x51,
    0x50, 0x44, 0x36, 0x3a, 0x21, 0x51, 0x4f, 0x2b, 0x3c, 0x40, 0x47, 0x73,
    0x3b, 0x61, 0x74, 0x48, 0x4b, 0x38, 0x21, 0x37, 0x49, 0x68, 0x52, 0x42,
    0x70, 0x67, 0x74, 0x2d, 0x57, 0x43, 0x37, 0x2d, 0x74, 0x52, 0x30, 0x42,
    0x51, 0x4d, 0x52, 0x3c, 0x62, 0x63, 0x58, 0x24, 0x47, 0x65, 0x50, 0x73,
    0x24, 0x4e, 0x53, 0x46, 0x32, 0x30, 0x5e, 0x5c, 0x7e, 0x3e,
};

// clang-format on

// The shared plain text that both flate_encoded and cascade_encoded decode to.
static const char shared_plain[] =
    "Streams decode through the filter pipeline. "
    "Streams decode through the filter pipeline. "
    "Streams decode through the filter pipeline. ";

typedef struct TestFixture {
    CosStreamObjNode *node;
    CosData *decoded;
} TestFixture;

static bool
setup(TestFixture *fixture)
{
    fixture->node = NULL;
    fixture->decoded = NULL;
    return true;
}

static void
teardown(TestFixture *fixture)
{
    if (fixture->decoded) {
        cos_data_free(fixture->decoded);
        fixture->decoded = NULL;
    }
    if (fixture->node) {
        cos_stream_obj_node_destroy(fixture->node);
        fixture->node = NULL;
    }
}

static CosNameObjNode *
make_name(const char *value)
{
    return cos_name_obj_node_alloc(cos_string_alloc_with_str(value));
}

static CosData *
make_data(const unsigned char *bytes, size_t size)
{
    CosData * const data = cos_data_alloc(size);
    if (data && size > 0) {
        cos_data_append(data, bytes, size, NULL);
    }
    return data;
}

// Builds a stream node with the given encoded bytes and optional /Filter value
// (a name or array node, ownership transferred to the node).
static CosStreamObjNode *
make_stream(const unsigned char *encoded,
            size_t encoded_size,
            CosObjNode * COS_Nullable filter_value)
{
    CosDictObjNode * const dict = cos_dict_obj_node_create(NULL);
    if (filter_value) {
        cos_dict_obj_node_set(dict, make_name("Filter"), filter_value, NULL);
    }
    return cos_stream_obj_node_create(dict, make_data(encoded, encoded_size));
}

static bool
decoded_equals(const CosData *data, const char *expected, size_t expected_size)
{
    return data != NULL &&
           data->size == expected_size &&
           memcmp(data->bytes, expected, expected_size) == 0;
}

TEST_CASE_BEGIN(decode_passthrough_no_filter)
{
    fixture->node = make_stream((const unsigned char *)shared_plain,
                                sizeof(shared_plain) - 1,
                                NULL);

    fixture->decoded = cos_stream_obj_node_get_decoded_data(fixture->node, NULL);

    if (!decoded_equals(fixture->decoded, shared_plain, sizeof(shared_plain) - 1)) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}

TEST_CASE_END

TEST_CASE_BEGIN(decode_single_flate)
{
    fixture->node = make_stream(flate_encoded,
                                sizeof(flate_encoded),
                                (CosObjNode *)make_name("FlateDecode"));

    fixture->decoded = cos_stream_obj_node_get_decoded_data(fixture->node, NULL);

    if (!decoded_equals(fixture->decoded, shared_plain, sizeof(shared_plain) - 1)) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}

TEST_CASE_END

// The "Fl" abbreviation must resolve to the same filter as "FlateDecode".
TEST_CASE_BEGIN(decode_flate_abbreviation)
{
    fixture->node = make_stream(flate_encoded,
                                sizeof(flate_encoded),
                                (CosObjNode *)make_name("Fl"));

    fixture->decoded = cos_stream_obj_node_get_decoded_data(fixture->node, NULL);

    if (!decoded_equals(fixture->decoded, shared_plain, sizeof(shared_plain) - 1)) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}

TEST_CASE_END

TEST_CASE_BEGIN(decode_single_ascii_hex)
{
    fixture->node = make_stream(ascii_hex_encoded,
                                sizeof(ascii_hex_encoded),
                                (CosObjNode *)make_name("ASCIIHexDecode"));

    fixture->decoded = cos_stream_obj_node_get_decoded_data(fixture->node, NULL);

    if (!decoded_equals(fixture->decoded, ascii_hex_plain, sizeof(ascii_hex_plain) - 1)) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}

TEST_CASE_END

// A [ASCII85Decode FlateDecode] chain applied in order to doubly-encoded data.
TEST_CASE_BEGIN(decode_cascade)
{
    CosArrayObjNode * const filters = cos_array_obj_node_alloc(NULL);
    cos_array_obj_node_append(filters, (CosObjNode *)make_name("ASCII85Decode"), NULL);
    cos_array_obj_node_append(filters, (CosObjNode *)make_name("FlateDecode"), NULL);

    fixture->node = make_stream(cascade_encoded,
                                sizeof(cascade_encoded),
                                (CosObjNode *)filters);

    fixture->decoded = cos_stream_obj_node_get_decoded_data(fixture->node, NULL);

    if (!decoded_equals(fixture->decoded, shared_plain, sizeof(shared_plain) - 1)) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}

TEST_CASE_END

// An unsupported filter must fail with COS_ERROR_NOT_IMPLEMENTED, not silently.
TEST_CASE_BEGIN(decode_unsupported_filter)
{
    fixture->node = make_stream(flate_encoded,
                                sizeof(flate_encoded),
                                (CosObjNode *)make_name("LZWDecode"));

    CosError error = {COS_ERROR_NONE, NULL};
    fixture->decoded = cos_stream_obj_node_get_decoded_data(fixture->node, &error);

    if (fixture->decoded != NULL || error.code != COS_ERROR_NOT_IMPLEMENTED) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}

TEST_CASE_END

// A /DecodeParms predictor cannot yet be applied, so decoding must be refused
// rather than returning mis-decoded bytes.
TEST_CASE_BEGIN(decode_predictor_guard)
{
    CosDictObjNode * const parms = cos_dict_obj_node_create(NULL);
    cos_dict_obj_node_set(parms,
                          make_name("Predictor"),
                          (CosObjNode *)cos_int_obj_node_alloc(12),
                          NULL);

    CosDictObjNode * const dict = cos_dict_obj_node_create(NULL);
    cos_dict_obj_node_set(dict, make_name("Filter"), (CosObjNode *)make_name("FlateDecode"), NULL);
    cos_dict_obj_node_set(dict, make_name("DecodeParms"), (CosObjNode *)parms, NULL);
    fixture->node = cos_stream_obj_node_create(dict,
                                               make_data(flate_encoded, sizeof(flate_encoded)));

    CosError error = {COS_ERROR_NONE, NULL};
    fixture->decoded = cos_stream_obj_node_get_decoded_data(fixture->node, &error);

    if (fixture->decoded != NULL || error.code != COS_ERROR_NOT_IMPLEMENTED) {
        TEST_FAILURE();
    }

    TEST_SUCCESS();
}

TEST_CASE_END

TEST_MAIN()
{
    TestFixture fixture = {0};

    TEST_RUN(decode_passthrough_no_filter, &fixture);
    TEST_RUN(decode_single_flate, &fixture);
    TEST_RUN(decode_flate_abbreviation, &fixture);
    TEST_RUN(decode_single_ascii_hex, &fixture);
    TEST_RUN(decode_cascade, &fixture);
    TEST_RUN(decode_unsupported_filter, &fixture);
    TEST_RUN(decode_predictor_guard, &fixture);

    return EXIT_SUCCESS;
}
