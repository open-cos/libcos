//
// Created by david on 17/12/23.
//

#include "libcos/common/CosError.h"
#include "libcos/io/CosFileInputStream.h"
#include "libcos/syntax/CosKeywords.h"
#include "parse/CosTokenizer.h"

#include <libcos/io/CosInputStream.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

extern int mainxx(int argc, char * COS_Nonnull argv[]);

int
mainxx(int argc, char * COS_Nonnull argv[])
{
    // Get the current working directory.

    CosFileInputStream * const input_stream = cos_file_input_stream_open("/home/david/Projects/C/libcos/tests/data/Hello-world.pdf",
                                                                         "r",
                                                                         NULL);
    if (!input_stream) {
        goto failure;
    }

    CosTokenizer * const tokenizer = cos_tokenizer_alloc((CosInputStream *)input_stream);
    if (!tokenizer) {
        goto failure;
    }

    while (cos_tokenizer_has_next_token(tokenizer)) {
        CosToken token = {0};
        CosTokenValue *token_value = NULL;
        CosError error = {0};

        if (!cos_tokenizer_get_next_token(tokenizer,
                                          &token,
                                          &token_value,
                                          &error)) {
            goto failure;
        }

        if (token_value) {
            cos_token_value_free(token_value);
        }
    }

    while (cos_tokenizer_has_next_token(tokenizer)) {
        const CosToken token = cos_tokenizer_next_token(tokenizer);
        switch (token.type) {
            case CosToken_Type_Unknown: {
                const CosString *string = NULL;
                if (cos_token_value_get_string(token.value,
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
                if (cos_token_value_get_data(token.value,
                                             &data)) {
                    printf("Literal String: %.*s\n",
                           (int)data->size,
                           data->bytes);
                }
            } break;
            case CosToken_Type_Hex_String: {
                const CosData *data = NULL;
                if (cos_token_value_get_data(token.value,
                                             &data)) {
                    printf("Hex String: %.*s\n",
                           (int)data->size,
                           data->bytes);
                }
            } break;

            case CosToken_Type_Name: {
                const CosString *string = NULL;
                if (cos_token_value_get_string(token.value,
                                               &string)) {
                    printf("Name: %s\n",
                           cos_string_get_data(string));
                }
            } break;

            case CosToken_Type_Integer: {
                int value = 0;
                if (cos_token_value_get_integer_number(token.value,
                                                       &value)) {
                    printf("Integer: %d\n",
                           value);
                }
            } break;
            case CosToken_Type_Real: {
                double value = 0.0;
                if (cos_token_value_get_real_number(token.value,
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

            case CosToken_Type_Keyword: {
                CosKeywordType value = CosKeywordType_Unknown;
                if (cos_token_value_get_keyword(token.value,
                                                &value)) {
                    printf("Keyword: %s\n",
                           cos_keyword_type_to_string(value));
                }
            } break;

            case CosToken_Type_EOF: {
                printf("EOF\n");
            } break;
        }
    }

    cos_tokenizer_free(tokenizer);
    cos_input_stream_close((CosInputStream *)input_stream);

    return EXIT_SUCCESS;

failure:
    return EXIT_FAILURE;
}

COS_ASSUME_NONNULL_END
