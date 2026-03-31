/*
 * Copyright (c) 2025 OpenCOS.
 *
 * Example: Traverse objects in a PDF document using the high-level CosObj API.
 *
 * Demonstrates how to:
 *   - Open and parse a PDF document from memory.
 *   - Wrap the document root in a CosObj.
 *   - Query types and values without manual type-casting.
 *   - Iterate over dictionary entries using CosObjDictIterator.
 *   - Access array elements using cos_obj_get_object_at_index.
 *   - Observe that indirect and reference objects resolve transparently.
 */

#include "CosExample.h"

#include <libcos/CosDoc.h>
#include <libcos/CosObj.h>
#include <libcos/CosParser.h>
#include <libcos/common/CosError.h>
#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>
#include <libcos/objects/CosObjNode.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

// MARK: - Helpers

static const char *
obj_type_name_(CosObjType type)
{
    switch (type) {
        case CosObjType_Boolean:
            return "Boolean";
        case CosObjType_Integer:
            return "Integer";
        case CosObjType_Real:
            return "Real";
        case CosObjType_String:
            return "String";
        case CosObjType_Name:
            return "Name";
        case CosObjType_Array:
            return "Array";
        case CosObjType_Dict:
            return "Dict";
        case CosObjType_Stream:
            return "Stream";
        case CosObjType_Null:
            return "Null";
        case CosObjType_Unknown:
            return "Unknown";
    }
    return "Unknown";
}

static void
print_indent_(int depth)
{
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
}

// MARK: - Object traversal

/* Forward declaration. */
static void
visit_obj_(CosObj *obj, int depth);

static void
visit_dict_(CosObj *obj, int depth)
{
    print_indent_(depth);
    printf("Dict (%zu entries):\n", cos_obj_get_dict_count(obj));

    CosObjDictIterator iter = cos_obj_dict_iterator_init(obj);
    const char *key = NULL;
    CosObj *value = NULL;

    while (cos_obj_dict_iterator_next(&iter, &key, &value)) {
        print_indent_(depth + 1);
        printf("/%s ->", key ? key : "");

        /* Skip /Parent to avoid infinite recursion in the page tree. */
        if (key && strcmp(key, "Parent") == 0) {
            printf(" (%s, skipped)\n", obj_type_name_(cos_obj_get_type(value)));
        }
        else {
            printf("\n");
            visit_obj_(value, depth + 2);
        }

        cos_obj_destroy(value);
        value = NULL;
    }
}

static void
visit_array_(CosObj *obj, int depth)
{
    const size_t count = cos_obj_get_array_count(obj);

    print_indent_(depth);
    printf("Array (%zu elements):\n", count);

    for (size_t i = 0; i < count; i++) {
        CosError error = cos_error_none();
        CosObj *element = cos_obj_get_object_at_index(obj, i, &error);
        if (!element) {
            print_indent_(depth + 1);
            printf("[%zu] <error>\n", i);
            continue;
        }

        print_indent_(depth + 1);
        printf("[%zu]\n", i);
        visit_obj_(element, depth + 2);

        cos_obj_destroy(element);
    }
}

static void
visit_obj_(CosObj *obj, int depth)
{
    const CosObjType type = cos_obj_get_type(obj);

    /* Indirect objects resolve transparently -- just note it for illustration. */
    if (cos_obj_is_indirect(obj)) {
        print_indent_(depth);
        printf("(indirect) ");
        depth++;
    }

    switch (type) {
        case CosObjType_Dict:
            visit_dict_(obj, depth);
            break;

        case CosObjType_Array:
            visit_array_(obj, depth);
            break;

        case CosObjType_Integer:
            print_indent_(depth);
            printf("Integer: %d\n", cos_obj_get_int_value(obj));
            break;

        case CosObjType_Real:
            print_indent_(depth);
            printf("Real: %g\n", cos_obj_get_real_value(obj));
            break;

        case CosObjType_Boolean:
            print_indent_(depth);
            printf("Boolean: %s\n", cos_obj_get_bool_value(obj) ? "true" : "false");
            break;

        case CosObjType_Name: {
            const CosString *str = cos_obj_get_name_value(obj);
            print_indent_(depth);
            printf("Name: /%s\n", (str && cos_string_get_data(str)) ? cos_string_get_data(str) : "");
            break;
        }

        case CosObjType_Null:
            print_indent_(depth);
            printf("Null\n");
            break;

        case CosObjType_Unknown:
        case CosObjType_String:
        case CosObjType_Stream:
            print_indent_(depth);
            printf("%s\n", obj_type_name_(type));
            break;
    }
}

// MARK: - Test PDF

/*
 * Minimal PDF with a catalog, a pages tree, and one empty page.
 *
 * Object layout:
 *   1 0 obj  << /Type /Catalog /Pages 2 0 R >>    @ offset 9
 *   2 0 obj  << /Type /Pages /Kids [3 0 R] /Count 1 >>    @ offset 58
 *   3 0 obj  << /Type /Page /Parent 2 0 R /MediaBox [0 0 612 792] >>    @ offset 115
 *   xref    @ offset 186
 */
static const char k_test_pdf[] =
    "%PDF-1.4\n"
    "1 0 obj\n"
    "<< /Type /Catalog /Pages 2 0 R >>\n"
    "endobj\n"
    "2 0 obj\n"
    "<< /Type /Pages /Kids [3 0 R] /Count 1 >>\n"
    "endobj\n"
    "3 0 obj\n"
    "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 612 792] >>\n"
    "endobj\n"
    "xref\n"
    "0 4\n"
    "0000000000 65535 f \n"
    "0000000009 00000 n \n"
    "0000000058 00000 n \n"
    "0000000115 00000 n \n"
    "trailer\n"
    "<< /Size 4 /Root 1 0 R >>\n"
    "startxref\n"
    "186\n"
    "%%EOF";

// MARK: - Entry point

EXAMPLE_MAIN()
{
    CosDoc *doc = NULL;
    CosMemoryStream *stream = NULL;
    CosObj *root_obj = NULL;
    CosError error = cos_error_none();
    int result = EXIT_FAILURE;

    doc = cos_doc_create(NULL);
    if (!doc) {
        fprintf(stderr, "Failed to create document\n");
        goto cleanup;
    }

    stream = cos_memory_stream_create_readonly(k_test_pdf, strlen(k_test_pdf));
    if (!stream) {
        fprintf(stderr, "Failed to create memory stream\n");
        goto cleanup;
    }

    CosParser *parser = cos_parser_create(doc, (CosStream *)stream);
    if (!parser) {
        fprintf(stderr, "Failed to create parser\n");
        goto cleanup;
    }

    if (!cos_parser_parse(parser, &error)) {
        fprintf(stderr, "Parse failed (error %d)\n", (int)error.code);
        goto cleanup;
    }

    printf("PDF version: 1.%d\n\n", cos_doc_get_version(doc));

    CosObjNode *root_node = cos_doc_get_root(doc);
    if (!root_node) {
        fprintf(stderr, "No root object found\n");
        goto cleanup;
    }

    root_obj = cos_obj_create(root_node);
    if (!root_obj) {
        fprintf(stderr, "Failed to wrap root object\n");
        goto cleanup;
    }

    printf("--- Document object tree ---\n");
    visit_obj_(root_obj, 0);
    printf("----------------------------\n");

    result = EXIT_SUCCESS;

cleanup:
    if (root_obj) {
        cos_obj_destroy(root_obj);
    }
    if (doc) {
        cos_doc_destroy(doc);
    }
    if (stream) {
        cos_stream_close((CosStream *)stream);
    }

    return result;
}

COS_ASSUME_NONNULL_END
