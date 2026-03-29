/*
 * Copyright (c) 2025 OpenCOS.
 *
 * Example: Iterate over objects in a PDF document.
 *
 * Demonstrates how to:
 *   - Open and parse a PDF document from memory.
 *   - Retrieve the document root (catalog dictionary).
 *   - Iterate over dictionary entries using CosDictObjIterator.
 *   - Resolve indirect object references.
 *   - Recursively walk the object tree.
 */

#include "CosExample.h"

#include <libcos/CosDoc.h>
#include <libcos/CosObjID.h>
#include <libcos/CosParser.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosString.h>
#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>
#include <libcos/objects/CosArrayObj.h>
#include <libcos/objects/CosDictObj.h>
#include <libcos/objects/CosIndirectObj.h>
#include <libcos/objects/CosIntObj.h>
#include <libcos/objects/CosNameObj.h>
#include <libcos/objects/CosObj.h>
#include <libcos/objects/CosReferenceObj.h>

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
        case CosObjType_Indirect:
            return "Indirect";
        case CosObjType_Reference:
            return "Reference";
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

static void
visit_object_(CosObj *obj, int depth);

static void
visit_dict_(CosDictObj *dict_obj, int depth)
{
    const size_t count = cos_dict_obj_get_count(dict_obj);

    print_indent_(depth);
    printf("Dict (%zu entries):\n", count);

    CosDictObjIterator iter = cos_dict_obj_iterator_init(dict_obj);
    CosNameObj *key = NULL;
    CosObj *value = NULL;

    while (cos_dict_obj_iterator_next(&iter, &key, &value)) {
        const CosString *key_str = cos_name_obj_get_value(key);
        const char *key_name = key_str ? cos_string_get_data(key_str) : "";

        print_indent_(depth + 1);
        printf("/%s -> ", key_name);

        /* Skip /Parent to avoid infinite recursion in the page tree. */
        if (strcmp(key_name, "Parent") == 0) {
            printf("(%s, skipped)\n", obj_type_name_(cos_obj_get_type(value)));
        }
        else {
            printf("\n");
            visit_object_(value, depth + 2);
        }
    }
}

static void
visit_array_(CosArrayObj *array_obj, int depth)
{
    CosError error = cos_error_none();
    const size_t count = cos_array_obj_get_count(array_obj);

    print_indent_(depth);
    printf("Array (%zu elements):\n", count);

    for (size_t i = 0; i < count; i++) {
        CosObj *element = cos_array_obj_get_at(array_obj, i, &error);
        if (!element) {
            print_indent_(depth + 1);
            printf("[%zu] <error>\n", i);
            continue;
        }

        print_indent_(depth + 1);
        printf("[%zu]\n", i);
        visit_object_(element, depth + 2);
    }
}

static void
visit_object_(CosObj *obj, int depth)
{
    const CosObjType type = cos_obj_get_type(obj);

    switch (type) {
        case CosObjType_Dict:
            visit_dict_((CosDictObj *)obj, depth);
            break;

        case CosObjType_Array:
            visit_array_((CosArrayObj *)obj, depth);
            break;

        case CosObjType_Indirect: {
            CosIndirectObj *indirect = (CosIndirectObj *)obj;
            CosObjID id = cos_indirect_obj_get_id(indirect);
            print_indent_(depth);
            printf("Indirect object %u %u:\n", id.obj_number, id.gen_number);

            CosObj *value = cos_indirect_obj_get_value(indirect);
            if (value) {
                visit_object_(value, depth + 1);
            }
            break;
        }

        case CosObjType_Reference: {
            CosReferenceObj *ref = (CosReferenceObj *)obj;
            print_indent_(depth);
            printf("Reference -> resolving\n");

            CosObj *resolved = cos_reference_obj_get_value(ref);
            if (resolved) {
                visit_object_(resolved, depth + 1);
            }
            else {
                print_indent_(depth + 1);
                printf("<unresolved>\n");
            }
            break;
        }

        case CosObjType_Integer: {
            int value = cos_int_obj_get_value((CosIntObj *)obj);
            print_indent_(depth);
            printf("Integer: %d\n", value);
            break;
        }

        case CosObjType_Name: {
            const CosString *str = cos_name_obj_get_value((CosNameObj *)obj);
            const char *data = str ? cos_string_get_data(str) : "";
            print_indent_(depth);
            printf("Name: /%s\n", data);
            break;
        }

        case CosObjType_Unknown:
        case CosObjType_Boolean:
        case CosObjType_Real:
        case CosObjType_String:
        case CosObjType_Stream:
        case CosObjType_Null:
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
    CosError error = cos_error_none();
    int result = EXIT_FAILURE;

    /* Create the document. */
    doc = cos_doc_create(NULL);
    if (!doc) {
        fprintf(stderr, "Failed to create document\n");
        goto cleanup;
    }

    /* Wrap the PDF bytes in a memory stream. */
    stream = cos_memory_stream_create_readonly(k_test_pdf, strlen(k_test_pdf));
    if (!stream) {
        fprintf(stderr, "Failed to create memory stream\n");
        goto cleanup;
    }

    /* Create the parser and parse the document.
     * The parser is owned by the document after creation. */
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

    /* Get the root object and walk the tree. */
    CosObj *root = cos_doc_get_root(doc);
    if (!root) {
        fprintf(stderr, "No root object found\n");
        goto cleanup;
    }

    printf("--- Document object tree ---\n");
    visit_object_(root, 0);
    printf("----------------------------\n");

    result = EXIT_SUCCESS;

cleanup:
    if (doc) {
        cos_doc_destroy(doc);
    }
    if (stream) {
        cos_stream_close((CosStream *)stream);
    }

    return result;
}

COS_ASSUME_NONNULL_END
