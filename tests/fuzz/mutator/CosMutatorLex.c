/*
 * Copyright (c) 2026 OpenCOS.
 */

#include "CosMutatorLex.h"

#include "common/Assert.h"

#include <string.h>

COS_ASSUME_NONNULL_BEGIN

static bool
cos_mut_is_whitespace_(unsigned char c)
{
    /* The COS whitespace set, which includes NUL. */
    return (c == 0x00 || c == 0x09 || c == 0x0A ||
            c == 0x0C || c == 0x0D || c == 0x20);
}

static bool
cos_mut_is_delimiter_(unsigned char c)
{
    return (c == '(' || c == ')' || c == '<' || c == '>' ||
            c == '[' || c == ']' || c == '{' || c == '}' ||
            c == '/' || c == '%');
}

static bool
cos_mut_is_regular_(unsigned char c)
{
    return !cos_mut_is_whitespace_(c) && !cos_mut_is_delimiter_(c);
}

static bool
cos_mut_is_digit_(unsigned char c)
{
    return (c >= '0' && c <= '9');
}

static bool
cos_mut_is_number_char_(unsigned char c)
{
    return cos_mut_is_digit_(c) || c == '+' || c == '-' || c == '.';
}

static bool
cos_mut_is_alpha_(unsigned char c)
{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

/**
 * Classifies an alphabetic run against the COS keyword set.
 */
static CosMutKeyword
cos_mut_lex_classify_keyword_(const unsigned char *data,
                              size_t length)
{
    COS_IMPL_PARAM_CHECK(data != NULL);

    static const struct {
        const char *text;
        size_t length;
        CosMutKeyword keyword;
    } keywords[] = {
        {"obj", 3, CosMutKeyword_Obj},
        {"endobj", 6, CosMutKeyword_EndObj},
        {"stream", 6, CosMutKeyword_Stream},
        {"endstream", 9, CosMutKeyword_EndStream},
        {"xref", 4, CosMutKeyword_XRef},
        {"trailer", 7, CosMutKeyword_Trailer},
        {"startxref", 9, CosMutKeyword_StartXRef},
        {"R", 1, CosMutKeyword_R},
        {"n", 1, CosMutKeyword_N},
        {"f", 1, CosMutKeyword_F},
        {"true", 4, CosMutKeyword_True},
        {"false", 5, CosMutKeyword_False},
        {"null", 4, CosMutKeyword_Null},
    };

    for (size_t i = 0; i < (sizeof(keywords) / sizeof(keywords[0])); i++) {
        if (keywords[i].length == length &&
            memcmp(data, keywords[i].text, length) == 0) {
            return keywords[i].keyword;
        }
    }

    return CosMutKeyword_Other;
}

/**
 * Finds @p needle in @p data at or after @p from.
 *
 * @return The offset of the match, or @p size when there is none.
 */
static size_t
cos_mut_lex_find_(const unsigned char *data,
                  size_t size,
                  size_t from,
                  const char *needle,
                  size_t needle_size)
{
    COS_IMPL_PARAM_CHECK(data != NULL);
    COS_IMPL_PARAM_CHECK(needle != NULL);

    if (needle_size == 0 || size < needle_size) {
        return size;
    }

    for (size_t i = from; i <= (size - needle_size); i++) {
        if (memcmp(data + i, needle, needle_size) == 0) {
            return i;
        }
    }

    return size;
}

/**
 * Scans a literal string, tolerating unterminated and over-nested input.
 *
 * @return The offset one past the string.
 */
static size_t
cos_mut_lex_scan_literal_string_(const unsigned char *data,
                                 size_t size,
                                 size_t start)
{
    COS_IMPL_PARAM_CHECK(data != NULL);

    size_t depth = 1;
    size_t i = start + 1;

    while (i < size) {
        const unsigned char c = data[i];

        if (c == '\\') {
            /* Skip the escaped byte; a trailing backslash just ends the scan. */
            i += 2;
            continue;
        }

        i++;

        if (c == '(') {
            depth++;
        }
        else if (c == ')') {
            depth--;
            if (depth == 0) {
                break;
            }
        }
    }

    return (i < size) ? i : size;
}

/**
 * Scans a hex string, tolerating a missing '>'.
 */
static size_t
cos_mut_lex_scan_hex_string_(const unsigned char *data,
                             size_t size,
                             size_t start)
{
    COS_IMPL_PARAM_CHECK(data != NULL);

    size_t i = start + 1;

    while (i < size && data[i] != '>') {
        i++;
    }

    /* Consume the terminator when there is one. */
    return (i < size) ? (i + 1) : size;
}

size_t
cos_mut_lex_scan_(const unsigned char *data,
                  size_t size,
                  CosMutSpan *spans,
                  size_t max_spans)
{
    COS_IMPL_PARAM_CHECK(data != NULL);
    COS_IMPL_PARAM_CHECK(spans != NULL);

    size_t count = 0;
    size_t pos = 0;

    while (pos < size) {
        /*
         * Reserve the last slot so that a truncated scan can still cover the
         * remainder of the input, keeping the tiling invariant intact.
         */
        if (count + 1 >= max_spans) {
            spans[count].offset = (uint32_t)pos;
            spans[count].length = (uint32_t)(size - pos);
            spans[count].type = (uint16_t)CosMutSpanType_Garbage;
            spans[count].keyword = (uint16_t)CosMutKeyword_None;
            count++;
            break;
        }

        const unsigned char c = data[pos];

        size_t end = pos + 1;
        CosMutSpanType type = CosMutSpanType_Garbage;
        CosMutKeyword keyword = CosMutKeyword_None;

        if (cos_mut_is_whitespace_(c)) {
            while (end < size && cos_mut_is_whitespace_(data[end])) {
                end++;
            }
            type = CosMutSpanType_Whitespace;
        }
        else if (c == '%') {
            while (end < size && data[end] != '\n' && data[end] != '\r') {
                end++;
            }
            type = CosMutSpanType_Comment;
        }
        else if (c == '(') {
            end = cos_mut_lex_scan_literal_string_(data, size, pos);
            type = CosMutSpanType_LiteralString;
        }
        else if (c == '<') {
            if ((pos + 1) < size && data[pos + 1] == '<') {
                end = pos + 2;
                type = CosMutSpanType_DictStart;
            }
            else {
                end = cos_mut_lex_scan_hex_string_(data, size, pos);
                type = CosMutSpanType_HexString;
            }
        }
        else if (c == '>') {
            if ((pos + 1) < size && data[pos + 1] == '>') {
                end = pos + 2;
                type = CosMutSpanType_DictEnd;
            }
            else {
                type = CosMutSpanType_Garbage;
            }
        }
        else if (c == '[') {
            type = CosMutSpanType_ArrayStart;
        }
        else if (c == ']') {
            type = CosMutSpanType_ArrayEnd;
        }
        else if (c == '/') {
            while (end < size && cos_mut_is_regular_(data[end])) {
                end++;
            }
            type = CosMutSpanType_Name;
        }
        else if (cos_mut_is_number_char_(c)) {
            bool is_real = (c == '.');
            while (end < size && cos_mut_is_number_char_(data[end])) {
                if (data[end] == '.') {
                    is_real = true;
                }
                end++;
            }
            type = is_real ? CosMutSpanType_Real : CosMutSpanType_Integer;
        }
        else if (cos_mut_is_alpha_(c)) {
            while (end < size && cos_mut_is_alpha_(data[end])) {
                end++;
            }
            type = CosMutSpanType_Keyword;
            keyword = cos_mut_lex_classify_keyword_(data + pos, end - pos);
        }

        spans[count].offset = (uint32_t)pos;
        spans[count].length = (uint32_t)(end - pos);
        spans[count].type = (uint16_t)type;
        spans[count].keyword = (uint16_t)keyword;
        count++;

        pos = end;

        if (keyword != CosMutKeyword_Stream) {
            continue;
        }

        /*
         * Stream payloads are taken as one opaque span. Lexing them would both
         * explode the span array on a large stream and invite the token
         * operators to shred compressed data, which is nearly always a wasted
         * mutation.
         */
        if (count + 2 >= max_spans) {
            continue;
        }

        /* The EOL after the 'stream' keyword is not part of the payload. */
        size_t eol_length = 0;
        if ((pos + 1) < size && data[pos] == '\r' && data[pos + 1] == '\n') {
            eol_length = 2;
        }
        else if (pos < size && (data[pos] == '\n' || data[pos] == '\r')) {
            eol_length = 1;
        }

        if (eol_length > 0) {
            spans[count].offset = (uint32_t)pos;
            spans[count].length = (uint32_t)eol_length;
            spans[count].type = (uint16_t)CosMutSpanType_Whitespace;
            spans[count].keyword = (uint16_t)CosMutKeyword_None;
            count++;
            pos += eol_length;
        }

        const size_t payload_end = cos_mut_lex_find_(data, size, pos,
                                                     "endstream", 9);
        if (payload_end > pos) {
            spans[count].offset = (uint32_t)pos;
            spans[count].length = (uint32_t)(payload_end - pos);
            spans[count].type = (uint16_t)CosMutSpanType_StreamData;
            spans[count].keyword = (uint16_t)CosMutKeyword_None;
            count++;
            pos = payload_end;
        }
    }

    return count;
}

void
cos_mut_view_build_(CosMutFileView *view,
                    const unsigned char *data,
                    size_t size,
                    const CosMutSpan *spans,
                    size_t span_count)
{
    COS_IMPL_PARAM_CHECK(view != NULL);
    COS_IMPL_PARAM_CHECK(data != NULL);
    COS_IMPL_PARAM_CHECK(spans != NULL);

    memset(view, 0, sizeof(*view));

    view->data = data;
    view->size = size;
    view->spans = spans;
    view->span_count = span_count;

    /* cos_parser_parse_header_ wants "%PDF-" plus a digit '.' digit version. */
    view->has_pdf_header = (size >= 8 &&
                            memcmp(data, "%PDF-", 5) == 0 &&
                            cos_mut_is_digit_(data[5]) &&
                            data[6] == '.' &&
                            cos_mut_is_digit_(data[7]));

    for (size_t i = 0; i < span_count; i++) {
        const CosMutSpan * const span = &spans[i];

        if (span->type == (uint16_t)CosMutSpanType_Keyword) {
            if (span->keyword == (uint16_t)CosMutKeyword_XRef) {
                view->xref_kw_offset = span->offset;
                view->has_xref_kw = true;
            }
            else if (span->keyword == (uint16_t)CosMutKeyword_StartXRef) {
                view->startxref_kw_offset = span->offset;
                view->has_startxref_kw = true;
            }
        }
        else if (span->type == (uint16_t)CosMutSpanType_Comment &&
                 span->length >= 5 &&
                 memcmp(data + span->offset, "%%EOF", 5) == 0) {
            view->eof_offset = span->offset;
            view->has_eof = true;
        }
    }
}

COS_ASSUME_NONNULL_END
