//
// Created by david on 12/07/23.
//

#ifndef LIBCOS_COMMON_COS_TYPES_H
#define LIBCOS_COMMON_COS_TYPES_H

typedef struct CosData CosData;
typedef struct CosDataBuffer CosDataBuffer;
typedef struct CosString CosString;
typedef struct CosStringRef CosStringRef;

typedef struct CosLexer CosLexer;
typedef struct CosParser CosParser;

typedef struct CosDocument CosDocument;
typedef struct CosObj CosObj;

typedef struct CosObjRef CosObjRef;
typedef struct CosIndirectObjectId CosIndirectObjectId;

typedef struct CosBoolObj CosBoolObj;
typedef struct CosNameObj CosNameObj;
typedef struct CosArrayObj CosArrayObj;
typedef struct CosDictObj CosDictObj;
typedef struct CosStringObj CosStringObj;

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

#endif /* LIBCOS_COMMON_COS_TYPES_H */
