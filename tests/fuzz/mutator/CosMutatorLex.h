/*
 * Copyright (c) 2026 OpenCOS.
 */

#ifndef LIBCOS_TESTS_FUZZ_COS_MUTATOR_LEX_H
#define LIBCOS_TESTS_FUZZ_COS_MUTATOR_LEX_H

#include <libcos/common/CosDefines.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * The upper bound on the span array.
 *
 * On overflow the lexer emits a single trailing @c Garbage span covering the
 * remainder of the input, so that the tiling invariant still holds for an input
 * at @c COS_FUZZ_MAX_INPUT_SIZE.
 */
#define COS_MUT_MAX_SPANS ((size_t)65536)

typedef enum CosMutSpanType {
    CosMutSpanType_Whitespace = 0,
    CosMutSpanType_Comment,
    CosMutSpanType_Integer,
    CosMutSpanType_Real,
    CosMutSpanType_Name,
    CosMutSpanType_LiteralString,
    CosMutSpanType_HexString,
    CosMutSpanType_ArrayStart,
    CosMutSpanType_ArrayEnd,
    CosMutSpanType_DictStart,
    CosMutSpanType_DictEnd,
    CosMutSpanType_Keyword,
    CosMutSpanType_StreamData,
    CosMutSpanType_Garbage,
} CosMutSpanType;

/**
 * Mirrors the keyword-typed members of @c CosToken_Type
 * (include/libcos/syntax/tokenizer/CosToken.h).
 */
typedef enum CosMutKeyword {
    CosMutKeyword_None = 0,
    CosMutKeyword_Obj,
    CosMutKeyword_EndObj,
    CosMutKeyword_Stream,
    CosMutKeyword_EndStream,
    CosMutKeyword_XRef,
    CosMutKeyword_Trailer,
    CosMutKeyword_StartXRef,
    CosMutKeyword_R,
    CosMutKeyword_N,
    CosMutKeyword_F,
    CosMutKeyword_True,
    CosMutKeyword_False,
    CosMutKeyword_Null,
    CosMutKeyword_Other,
} CosMutKeyword;

typedef struct CosMutSpan {
    uint32_t offset;
    uint32_t length;
    uint16_t type;    /* CosMutSpanType */
    uint16_t keyword; /* CosMutKeyword */
} CosMutSpan;

/**
 * The file-level landmarks a mutant needs in order to be repaired.
 *
 * The offsets are of the *last* occurrence of each landmark, matching
 * cos_parser_find_startxref_, which scans backwards from end-of-file and takes
 * the last %%EOF it finds.
 */
typedef struct CosMutFileView {
    const unsigned char * COS_Nullable data;
    size_t size;
    const CosMutSpan * COS_Nullable spans;
    size_t span_count;
    uint32_t xref_kw_offset;
    uint32_t startxref_kw_offset;
    uint32_t eof_offset;
    bool has_xref_kw;
    bool has_startxref_kw;
    bool has_eof;
    bool has_pdf_header;
} CosMutFileView;

/**
 * Splits @p data into spans, writing at most @p max_spans of them.
 *
 * The lexer is deliberately tolerant: unparseable bytes become @c Garbage spans
 * rather than ending the scan, so that a malformed region of a mutant is still
 * available as a mutation site. It recognizes token boundaries only -- it does
 * not decode values, apply limits, or report errors.
 *
 * The spans always tile the input: offsets strictly increase, each span begins
 * where the previous one ends, no span is empty, and the lengths sum to
 * @p size.
 *
 * @return The number of spans written.
 */
size_t
cos_mut_lex_scan_(const unsigned char *data,
                  size_t size,
                  CosMutSpan *spans,
                  size_t max_spans);

/**
 * Locates the file-level landmarks in an already-scanned span array.
 */
void
cos_mut_view_build_(CosMutFileView *view,
                    const unsigned char *data,
                    size_t size,
                    const CosMutSpan *spans,
                    size_t span_count)
    COS_ATTR_ACCESS_WRITE_ONLY(1);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_TESTS_FUZZ_COS_MUTATOR_LEX_H */
