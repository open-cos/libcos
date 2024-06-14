//
// Created by david on 17/12/23.
//

#include "syntax/tokenizer/CosTokenizer.h"

#include "libcos/common/CosError.h"
#include "libcos/io/CosFileInputStream.h"
#include "libcos/syntax/CosKeywords.h"

#include <libcos/io/CosFileStream.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

extern int
mainxx(int argc, char * COS_Nonnull argv[]);

int
mainxx(COS_ATTR_UNUSED int argc,
       COS_ATTR_UNUSED char * COS_Nonnull argv[])
{
    // Get the current working directory.

    CosStream * const input_stream = cos_file_stream_create("/home/david/Projects/C/libcos/tests/data/Hello-world.pdf",
                                                            "r");
    if (!input_stream) {
        goto failure;
    }

    CosTokenizer * const tokenizer = cos_tokenizer_alloc(input_stream);
    if (!tokenizer) {
        goto failure;
    }

    while (cos_tokenizer_has_next_token(tokenizer)) {
        CosError error = {0};
        CosToken *token = cos_tokenizer_get_next_token(tokenizer,
                                                       &error);

        if (!token) {
            goto failure;
        }

        switch (token->type) {
            case CosToken_Type_Unknown: {
                const CosString *string = NULL;
                if (cos_token_value_get_string(token->value,
                                               &string)) {
                    printf("Unknown: %s\n",
                           cos_string_get_data(string));
                }
                else {
                    printf("Unknown\n");
                }
            } break;

            case CosToken_Type_Literal_String: {
                const CosData *data = NULL;
                if (cos_token_value_get_data(token->value,
                                             &data)) {
                    printf("Literal String: %.*s\n",
                           (int)data->size,
                           data->bytes);
                }
            } break;
            case CosToken_Type_Hex_String: {
                const CosData *data = NULL;
                if (cos_token_value_get_data(token->value,
                                             &data)) {
                    printf("Hex String: %.*s\n",
                           (int)data->size,
                           data->bytes);
                }
            } break;

            case CosToken_Type_Name: {
                const CosString *string = NULL;
                if (cos_token_value_get_string(token->value,
                                               &string)) {
                    printf("Name: %s\n",
                           cos_string_get_data(string));
                }
            } break;

            case CosToken_Type_Integer: {
                int value = 0;
                if (cos_token_value_get_integer_number(token->value,
                                                       &value)) {
                    printf("Integer: %d\n",
                           value);
                }
            } break;
            case CosToken_Type_Real: {
                double value = 0.0;
                if (cos_token_value_get_real_number(token->value,
                                                    &value)) {
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
            } break;
        }

        cos_tokenizer_release_token(tokenizer, token);
    }

    cos_tokenizer_free(tokenizer);
    cos_stream_close(input_stream);

    return EXIT_SUCCESS;

failure:
    return EXIT_FAILURE;
}

COS_ASSUME_NONNULL_END
