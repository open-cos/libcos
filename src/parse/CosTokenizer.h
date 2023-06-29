//
// Created by david on 03/06/23.
//

#ifndef LIBCOS_COS_TOKENIZER_H
#define LIBCOS_COS_TOKENIZER_H

#include <libcos/io/CosInputStream.h>

struct CosTokenizer;
typedef struct CosTokenizer CosTokenizer;

struct CosToken;
typedef struct CosToken CosToken;

typedef enum CosToken_Type {
    CosToken_Type_Unknown,

    CosToken_Type_Boolean,
    CosToken_Type_Literal_String,
    CosToken_Type_Hex_String,
    CosToken_Type_Name,
    CosToken_Type_Integer,
    CosToken_Type_Real,
    CosToken_Type_Stream,
    CosToken_Type_Null,
    CosToken_Type_EndOfLine,

    CosToken_Type_ArrayStart,
    CosToken_Type_ArrayEnd,
    CosToken_Type_DictionaryStart,
    CosToken_Type_DictionaryEnd,

    CosToken_Type_Comment,

    CosToken_Type_EOF,
} CosToken_Type;

CosTokenizer *
cos_tokenizer_alloc(CosInputStream *input_stream);

void
cos_tokenizer_free(CosTokenizer *tokenizer);

/**
 * Get the input stream associated with the tokenizer.
 *
 * @param tokenizer The tokenizer.
 *
 * @return The input stream associated with the tokenizer.
 */
CosInputStream *
cos_tokenizer_get_input_stream(const CosTokenizer *tokenizer);

CosToken *
cos_tokenizer_next_token(CosTokenizer *tokenizer);

#endif /* LIBCOS_COS_TOKENIZER_H */
