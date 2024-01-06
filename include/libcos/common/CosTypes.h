//
// Created by david on 12/07/23.
//

#ifndef LIBCOS_COMMON_COS_TYPES_H
#define LIBCOS_COMMON_COS_TYPES_H

#include <libcos/common/CosDefines.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef struct CosObj CosObj;
typedef struct CosClass CosClass;

typedef struct CosError CosError;

typedef struct CosData CosData;
typedef struct CosDataRef CosDataRef;
typedef struct CosDataBuffer CosDataBuffer;
typedef struct CosString CosString;
typedef struct CosStringRef CosStringRef;
typedef struct CosArray CosArray;
typedef struct CosDict CosDict;

typedef struct CosTokenizer CosTokenizer;
typedef struct CosObjParser CosObjParser;

typedef struct CosDoc CosDoc;
typedef struct CosBaseObj CosBaseObj;

typedef struct CosObjRef CosObjRef;
typedef struct CosObjID CosObjID;

typedef struct CosBoolObj CosBoolObj;
typedef struct CosNameObj CosNameObj;
typedef struct CosArrayObj CosArrayObj;
typedef struct CosDictObj CosDictObj;
typedef struct CosStringObj CosStringObj;
typedef struct CosNullObj CosNullObj;
typedef struct CosIntegerObj CosIntegerObj;
typedef struct CosRealObj CosRealObj;

typedef struct CosIndirectObj CosIndirectObj;

typedef struct CosInputStream CosInputStream;
typedef struct CosInputStreamFunctions CosInputStreamFunctions;
typedef struct CosFileInputStream CosFileInputStream;

typedef struct CosBufferedInputStream CosBufferedInputStream;

typedef struct CosDiagnostic CosDiagnostic;
typedef struct CosDiagnosticHandler CosDiagnosticHandler;
typedef struct CosLogContext CosLogContext;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_TYPES_H */
