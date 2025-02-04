/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "syntax/tokenizer/CosTokenValue.h"
#include "syntax/tokenizer/CosTokenizer.h"

#include <libcos/common/CosError.h>
#include <libcos/io/CosFileStream.h>

#include <stdio.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

extern int
TEST_NAME(int argc, char * COS_Nonnull argv[]);

int
TEST_NAME(COS_ATTR_UNUSED int argc,
          COS_ATTR_UNUSED char * COS_Nonnull argv[])
{
    // Get the current working directory.
    CosStream *input_stream = NULL;
    CosTokenizer *tokenizer = NULL;

    input_stream = cos_file_stream_create("/home/david/Projects/C/libcos/tests/data/Hello-world.pdf",
                                          "r");
    if (!input_stream) {
        goto failure;
    }

    tokenizer = cos_tokenizer_create(input_stream);
    if (!tokenizer) {
        goto failure;
    }

    bool eof = false;
    do {
        CosToken token = {0};
        CosError error = {0};
        if (!cos_tokenizer_get_next_token(tokenizer,
                                          &token,
                                          &error)) {
            goto failure;
        }

        switch (token.type) {
            case CosToken_Type_Unknown: {
                if (token.value.type == CosTokenValue_Type_String) {
                    CosString * const string = token.value.value.string;
                    printf("Unknown: %s\n",
                           cos_string_get_data(string));
                }
                else {
                    printf("Unknown\n");
                }
            } break;

            case CosToken_Type_Literal_String: {
                if (token.value.type == CosTokenValue_Type_Data) {
                    const CosData * const data = token.value.value.data;
                    printf("Literal String: %.*s\n",
                           (int)data->size,
                           data->bytes);
                }
            } break;
            case CosToken_Type_Hex_String: {
                if (token.value.type == CosTokenValue_Type_Data) {
                    const CosData * const data = token.value.value.data;
                    printf("Hex String: %.*s\n",
                           (int)data->size,
                           data->bytes);
                }
            } break;

            case CosToken_Type_Name: {
                if (token.value.type == CosTokenValue_Type_String) {
                    const CosString * const string = token.value.value.string;
                    printf("Name: %s\n",
                           cos_string_get_data(string));
                }
            } break;

            case CosToken_Type_Integer: {
                if (token.value.type == CosTokenValue_Type_IntegerNumber) {
                    const int value = token.value.value.integer_number;
                    printf("Integer: %d\n",
                           value);
                }
            } break;
            case CosToken_Type_Real: {
                if (token.value.type == CosTokenValue_Type_RealNumber) {
                    const double value = token.value.value.real_number;
                    printf("Real: %f\n",
                           value);
                }
            } break;

            case CosToken_Type_ArrayStart: {
                printf("Array Start\n");
            } break;
            case CosToken_Type_ArrayEnd: {
                printf("Array End\n");
            } break;

            case CosToken_Type_DictionaryStart: {
                printf("Dictionary Start\n");
            } break;
            case CosToken_Type_DictionaryEnd: {
                printf("Dictionary End\n");
            } break;

            case CosToken_Type_True: {
                printf("True\n");
            } break;
            case CosToken_Type_False: {
                printf("False\n");
            } break;

            case CosToken_Type_Null: {
                printf("Null\n");
            } break;

            case CosToken_Type_R: {
                printf("R\n");
            } break;

            case CosToken_Type_Obj: {
                printf("Obj\n");
            } break;

            case CosToken_Type_EndObj: {
                printf("EndObj\n");
            } break;

            case CosToken_Type_Stream: {
                printf("Stream\n");
            } break;

            case CosToken_Type_EndStream: {
                printf("EndStream\n");
            } break;

            case CosToken_Type_XRef: {
                printf("XRef\n");
            } break;

            case CosToken_Type_N: {
                printf("N\n");
            } break;

            case CosToken_Type_F: {
                printf("F\n");
            } break;

            case CosToken_Type_Trailer: {
                printf("Trailer\n");
            } break;

            case CosToken_Type_StartXRef: {
                printf("StartXRef\n");
            } break;

            case CosToken_Type_EOF: {
                printf("EOF\n");
                eof = true;
            } break;
        }

        cos_token_reset(&token);
    } while (!eof);

    int result = EXIT_SUCCESS;
    goto cleanup;

failure:
    printf("Failed to parse the input stream\n");
    result = EXIT_FAILURE;

cleanup:
    if (tokenizer) {
        cos_tokenizer_destroy(tokenizer);
    }
    if (input_stream) {
        cos_stream_close(input_stream);
    }

    return result;
}

COS_ASSUME_NONNULL_END
