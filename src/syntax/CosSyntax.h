//
// Created by david on 17/12/23.
//

#ifndef LIBCOS_COS_SYNTAX_H
#define LIBCOS_COS_SYNTAX_H

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
    CosKeywordType_XRef,
    CosKeywordType_Trailer,
    CosKeywordType_StartXRef,
} CosKeywordType;

CosKeywordType
cos_keyword_type_from_string(CosStringRef string);

const char * COS_Nullable
cos_keyword_type_to_string(CosKeywordType keyword_type);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_SYNTAX_H */
