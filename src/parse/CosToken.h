//
// Created by david on 22/07/23.
//

#ifndef LIBCOS_COS_TOKEN_H
#define LIBCOS_COS_TOKEN_H

#include "common/Assert.h"
#include "common/CharacterSet.h"
#include "common/CosData.h"
#include "common/CosString.h"

#include <libcos/common/CosDefines.h>
#include <libcos/io/CosInputStream.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

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

    CosToken_Type_Keyword_True,
    CosToken_Type_Keyword_False,
    CosToken_Type_Keyword_Null,
    CosToken_Type_Keyword_R,
    CosToken_Type_Keyword_Obj,
    CosToken_Type_Keyword_EndObj,
    CosToken_Type_Keyword_Stream,
    CosToken_Type_Keyword_EndStream,
    CosToken_Type_Keyword_XRef,
    CosToken_Type_Keyword_Trailer,
    CosToken_Type_Keyword_StartXRef,

    CosToken_Type_EOF,
} CosToken_Type;

typedef struct CosTokenValue CosTokenValue;

struct CosToken {
    /**
     * @brief The type of the token.
     */
    CosToken_Type type;

    /**
     * @brief The text of the token.
     */
    CosStringRef text;

    /**
     * @brief The value of the token.
     *
     * This is only valid for certain token types.
     */
    CosTokenValue *value;
};

char *
cos_token_copy_string_value(const CosToken *token,
                            size_t *out_size);

bool
cos_token_get_string_value(const CosToken *token,
                           CosString **out_string);

bool
cos_token_get_data_value(const CosToken *token,
                         CosData **out_data);

bool
cos_token_get_boolean_value(const CosToken *token,
                            bool *value);

bool
cos_token_get_integer_value(const CosToken *token,
                            int *value);

bool
cos_token_get_real_value(const CosToken *token,
                         double *value);

#endif /* LIBCOS_COS_TOKEN_H */
