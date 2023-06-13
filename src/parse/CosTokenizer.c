//
// Created by david on 03/06/23.
//

#include "CosTokenizer.h"
#include "libcos/io/CosInputStream.h"

#include <stdio.h>
#include <stdlib.h>

struct CosTokenizer {
    CosInputStream *input_stream;
};

struct CosToken {
    CosToken_Type type;
    char *start;
    char *end;
    char *value;
};

static char *
cos_tokenizer_read_comment(CosTokenizer *tokenizer);

CosTokenizer *
cos_tokenizer_alloc(CosInputStream *input_stream)
{
    CosTokenizer * const tokenizer = malloc(sizeof(CosTokenizer));
    if (!tokenizer) {
        return NULL;
    }

    tokenizer->input_stream = input_stream;

    return tokenizer;
}

void
cos_tokenizer_free(CosTokenizer *tokenizer)
{
    free(tokenizer);
}

CosInputStream *
cos_tokenizer_get_input_stream(CosTokenizer *tokenizer)
{
    return tokenizer->input_stream;
}

CosToken *
cos_tokenizer_next_token(CosTokenizer *tokenizer)
{
    CosInputStream * const input_stream = tokenizer->input_stream;
    CosToken *token = malloc(sizeof(CosToken));
    if (!token) {
        return NULL;
    }

    token->type = CosToken_Type_Unknown;
    //    token->start = input_stream->current;
    //    token->end = input_stream->current;
    token->value = NULL;

    int c = cos_input_stream_getc(input_stream);
    if (c == EOF) {
        token->type = CosToken_Type_EOF;
        return token;
    }

    if (c == '[') {
        token->type = CosToken_Type_ArrayStart;
        return token;
    }

    if (c == ']') {
        token->type = CosToken_Type_ArrayEnd;
        return token;
    }

    if (c == '{') {
        token->type = CosToken_Type_DictionaryStart;
        return token;
    }

    if (c == '}') {
        token->type = CosToken_Type_DictionaryEnd;
        return token;
    }

    if (c == '/') {
        token->type = CosToken_Type_Name;
        token->value = cos_tokenizer_read_name(tokenizer);
        return token;
    }

    if (c == '(') {
        token->type = CosToken_Type_String;
        token->value = cos_tokenizer_read_literal_string(input_stream);
        return token;
    }

    //    if (c == ')') {
    //        token->type = CosToken_Type_StringEnd;
    //        return token;
    //    }

    if (c == '<') {
        c = cos_input_stream_getc(input_stream);
        if (c == '<') {
            token->type = CosToken_Type_DictionaryStart;
            return token;
        }

        token->type = CosToken_Type_String;
        token->value = cos_tokenizer_read_hex_string(tokenizer);
        return token;
    }

    if (c == '>') {
        c = cos_input_stream_getc(input_stream);
        if (c == '>') {
            token->type = CosToken_Type_DictionaryEnd;
            return token;
        }

        //        token->type = CosToken_Type_StringEnd;
        //        return token;
    }

    if (c == '%') {
        token->type = CosToken_Type_Comment;
        token->value = cos_tokenizer_read_comment(tokenizer);
        return token;
    }

    return token;
}

static char *
cos_tokenizer_read_comment(CosTokenizer *tokenizer)
{
    CosInputStream * const input_stream = tokenizer->input_stream;

    char *comment = NULL;
    int c = cos_input_stream_getc(input_stream);
    if (c == EOF) {
        return comment;
    }

    if (c == '%') {
        comment = cos_input_stream_read_line(input_stream);
    }

    return comment;
}
