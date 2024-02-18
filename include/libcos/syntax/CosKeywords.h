/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_SYNTAX_COS_KEYWORDS_H
#define LIBCOS_SYNTAX_COS_KEYWORDS_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef enum CosKeywordType {
    CosKeywordType_Unknown,

    CosKeywordType_True,
    CosKeywordType_False,
    CosKeywordType_Null,
    CosKeywordType_R,
    CosKeywordType_Obj,
    CosKeywordType_EndObj,
    CosKeywordType_Stream,
    CosKeywordType_EndStream,

    /**
     * @brief The "xref" keyword.
     *
     * This keyword is used to indicate the start of a cross-reference table.
     */
    CosKeywordType_XRef,

    /**
     * @brief The "n" keyword.
     *
     * This keyword is used to indicate in-use entries in a cross-reference table.
     */
    CosKeywordType_N,

    /**
     * @brief The "f" keyword.
     *
     * This keyword is used to indicate free entries in a cross-reference table.
     */
    CosKeywordType_F,

    CosKeywordType_Trailer,
    CosKeywordType_StartXRef,
} CosKeywordType;

/**
 * @brief Gets the keyword type from a string.
 *
 * @param string The string.
 *
 * @return The keyword type, or @c CosKeywordType_Unknown if the string does not
 * represent a keyword.
 */
CosKeywordType
cos_keyword_type_from_string(CosStringRef string);

/**
 * @brief Gets the string representation of a keyword type.
 *
 * @param keyword_type The keyword type.
 *
 * @return The string representation of the keyword type, or @c NULL if the
 * keyword type is unknown.
 */
const char * COS_Nullable
cos_keyword_type_to_string(CosKeywordType keyword_type);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_SYNTAX_COS_KEYWORDS_H */
