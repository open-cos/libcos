//
// Created by david on 12/07/23.
//

#ifndef LIBCOS_COMMON_COS_TYPES_H
#define LIBCOS_COMMON_COS_TYPES_H

#include <libcos/common/CosDefines.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

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

typedef struct CosScanner CosScanner;
typedef struct CosBaseParser CosBaseParser;

typedef struct CosToken CosToken;
typedef struct CosTokenValue CosTokenValue;

typedef struct CosTokenizer CosTokenizer;
typedef struct CosObjParser CosObjParser;

typedef struct CosDoc CosDoc;

typedef struct CosObjID CosObjID;

typedef struct CosNode CosNode;
typedef struct CosBoolNode CosBoolNode;
typedef struct CosIntegerNode CosIntegerNode;
typedef struct CosRealNode CosRealNode;
typedef struct CosStringNode CosStringNode;
typedef struct CosNameNode CosNameNode;
typedef struct CosArrayNode CosArrayNode;
typedef struct CosDictNode CosDictNode;
typedef struct CosStreamNode CosStreamNode;
typedef struct CosIndirectNode CosIndirectNode;
typedef struct CosReferenceNode CosReferenceNode;
typedef struct CosNullNode CosNullNode;

typedef struct CosObj CosObj;
typedef struct CosBoolObj CosBoolObj;
typedef struct CosIntObj CosIntObj;
typedef struct CosRealObj CosRealObj;
typedef struct CosStringObj CosStringObj;
typedef struct CosNameObj CosNameObj;
typedef struct CosArrayObj CosArrayObj;
typedef struct CosDictObj CosDictObj;
typedef struct CosStreamObj CosStreamObj;
typedef struct CosNullObj CosNullObj;
typedef struct CosIndirectObj CosIndirectObj;
typedef struct CosReferenceObj CosReferenceObj;

typedef struct CosXrefTable CosXrefTable;
typedef struct CosXrefSection CosXrefSection;
typedef struct CosXrefSubsection CosXrefSubsection;
typedef struct CosXrefEntry CosXrefEntry;

typedef struct CosXrefInUseEntry CosXrefInUseEntry;
typedef struct CosXrefFreeEntry CosXrefFreeEntry;

typedef struct CosXrefTableParser CosXrefTableParser;

typedef struct CosStream CosStream;
typedef struct CosFilter CosFilter;
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
