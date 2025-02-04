/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COS_TOKEN_H
#define LIBCOS_COS_TOKEN_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef enum CosToken_Type {
    CosToken_Type_Unknown,

    CosToken_Type_Literal_String,
    CosToken_Type_Hex_String,
    CosToken_Type_Name,
    CosToken_Type_Integer,
    CosToken_Type_Real,

    CosToken_Type_ArrayStart,
    CosToken_Type_ArrayEnd,
    CosToken_Type_DictionaryStart,
    CosToken_Type_DictionaryEnd,

    //    CosToken_Type_Keyword,

    CosToken_Type_True,
    CosToken_Type_False,
    CosToken_Type_Null,
    CosToken_Type_R,
    CosToken_Type_Obj,
    CosToken_Type_EndObj,
    CosToken_Type_Stream,
    CosToken_Type_EndStream,

    CosToken_Type_XRef,

    CosToken_Type_N,
    CosToken_Type_F,

    CosToken_Type_Trailer,
    CosToken_Type_StartXRef,

    CosToken_Type_EOF,
} CosToken_Type;

struct CosTokenValue {
    enum {
        CosTokenValue_Type_None = 0,

        CosTokenValue_Type_IntegerNumber,
        CosTokenValue_Type_LongIntegerNumber,
        CosTokenValue_Type_RealNumber,
        CosTokenValue_Type_String,
        CosTokenValue_Type_Data,
    } type;

    union {
        int integer_number;
        long long long_integer_number;
        double real_number;
        CosString *string;
        CosData *data;
    } value;
};

struct CosToken {
    /**
     * @brief The type of the token.
     */
    CosToken_Type type;

    /**
     * @brief The offset of the token in the input stream.
     */
    size_t offset;

    /**
     * @brief The length of the token in bytes.
     */
    size_t length;

    /**
     * @brief The value of the token.
     *
     * This is only valid for certain token types.
     */
    CosTokenValue value;
};

/**
 * @brief Resets a token to its initial state.
 *
 * @param token The token.
 */
void
cos_token_reset(CosToken *token);

bool
cos_token_get_integer_value(const CosToken *token,
                            int *out_value)
    COS_ATTR_ACCESS_WRITE_ONLY(2);

CosData * COS_Nullable
cos_token_move_data_value(CosToken *token)
    COS_OWNERSHIP_RETURNS
    COS_ATTR_ACCESS_READ_WRITE(1);

CosString * COS_Nullable
cos_token_move_string_value(CosToken *token)
    COS_OWNERSHIP_RETURNS
    COS_ATTR_ACCESS_READ_WRITE(1);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_TOKEN_H */
