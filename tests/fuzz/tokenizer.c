/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "CosFuzz.h"

#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>
#include <libcos/syntax/tokenizer/CosToken.h>
#include <libcos/syntax/tokenizer/CosTokenizer.h>

#include <stddef.h>
#include <stdint.h>

extern int
LLVMFuzzerTestOneInput(const uint8_t *data,
                       size_t size);

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

    CosTokenizer * const tokenizer = cos_tokenizer_create((CosStream *)stream);
    if (!tokenizer) {
        cos_stream_close((CosStream *)stream);
        return 0;
    }

    for (;;) {
        CosToken token = {0};

        if (!cos_tokenizer_get_next_token(tokenizer, &token, NULL)) {
            cos_token_reset(&token);
            break;
        }

        const CosToken_Type type = token.type;

        /* Frees any string or data value the token carries. */
        cos_token_reset(&token);

        if (type == CosToken_Type_EOF) {
            break;
        }
    }

    cos_tokenizer_destroy(tokenizer);
    cos_stream_close((CosStream *)stream);

    return 0;
}
