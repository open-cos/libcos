//
// Created by david on 14/11/23.
//

#ifndef LIBCOS_LEX_COS_LEXER_H
#define LIBCOS_LEX_COS_LEXER_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosTypes.h>
#include <libcos/io/CosInputStream.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef struct CosLocation CosLocation;
typedef struct CosToken CosToken;
typedef enum CosToken_Type CosToken_Type;

struct CosLocation {
    size_t offset;
};

struct CosByteRange {
    CosLocation start;
    size_t length;
};

enum CosToken_Type {
    CosToken_Type_Unknown = 0,

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
};

struct CosToken {
    CosToken_Type type;
    CosLocation location;
};

CosLexer * COS_Nullable
cos_lexer_alloc(CosDocument *document,
                CosInputStream *input_stream)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_lexer_free(CosLexer *lexer);

const CosInputStream *
cos_lexer_get_input_stream(const CosLexer *lexer);

bool
cos_lexer_next_token(CosLexer *lexer,
                     CosToken *token_out,
                     CosError ** COS_Nullable error_out);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_LEX_COS_LEXER_H */
