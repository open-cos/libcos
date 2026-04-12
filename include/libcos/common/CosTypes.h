//
// Created by david on 12/07/23.
//

#ifndef LIBCOS_COMMON_COS_TYPES_H
#define LIBCOS_COMMON_COS_TYPES_H

#include <libcos/common/CosDefines.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef struct CosObj CosObj;

typedef struct CosAllocator CosAllocator;

typedef struct CosError CosError;

typedef struct CosData CosData;
typedef struct CosDataRef CosDataRef;
typedef struct CosDataBuffer CosDataBuffer;

/**
 * @brief A nul-terminated string.
 */
typedef struct CosString CosString;
typedef struct CosStringRef CosStringRef;
typedef struct CosArray CosArray;
typedef struct CosDict CosDict;
typedef struct CosList CosList;
typedef struct CosRingBuffer CosRingBuffer;
typedef struct CosBasicStack CosBasicStack;
typedef CosBasicStack CosStack;

typedef struct CosNumber CosNumber;

typedef struct CosBaseParser CosBaseParser;
typedef struct CosParser CosParser;

typedef struct CosToken CosToken;
typedef struct CosTokenValue CosTokenValue;

typedef struct CosTokenizer CosTokenizer;
typedef struct CosObjParser CosObjParser;

typedef struct CosDoc CosDoc;

typedef struct CosObjID CosObjID;

typedef struct CosObjNode CosObjNode;
typedef struct CosBoolObjNode CosBoolObjNode;
typedef struct CosIntObjNode CosIntObjNode;
typedef struct CosRealObjNode CosRealObjNode;
typedef struct CosStringObjNode CosStringObjNode;
typedef struct CosNameObjNode CosNameObjNode;
typedef struct CosArrayObjNode CosArrayObjNode;
typedef struct CosDictObjNode CosDictObjNode;
typedef struct CosStreamObjNode CosStreamObjNode;
typedef struct CosNullObjNode CosNullObjNode;
typedef struct CosIndirectObjNode CosIndirectObjNode;
typedef struct CosReferenceObjNode CosReferenceObjNode;

typedef struct CosXrefTable CosXrefTable;
typedef struct CosXrefSection CosXrefSection;
typedef struct CosXrefSubsection CosXrefSubsection;
typedef struct CosXrefEntry CosXrefEntry;

typedef struct CosXrefInUseEntry CosXrefInUseEntry;
typedef struct CosXrefFreeEntry CosXrefFreeEntry;

typedef struct CosXrefTableParser CosXrefTableParser;

typedef struct CosStream CosStream;
typedef struct CosFilter CosFilter;
typedef struct CosFilterBuffer CosFilterBuffer;
typedef struct CosFilterFunctions CosFilterFunctions;
typedef struct CosASCII85Filter CosASCII85Filter;
typedef struct CosASCIIHexFilter CosASCIIHexFilter;
typedef struct CosRunLengthFilter CosRunLengthFilter;

typedef struct CosStreamReader CosStreamReader;

typedef struct CosDiagnostic CosDiagnostic;
typedef struct CosDiagnosticHandler CosDiagnosticHandler;

/**
 * @brief A logging context.
 */
typedef struct CosLogContext CosLogContext;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_TYPES_H */
