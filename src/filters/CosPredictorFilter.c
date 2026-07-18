/*
 * Copyright (c) 2025 OpenCOS.
 *
 * Reverses PDF /DecodeParms row prediction (PNG predictors 10-15 and the TIFF
 * Predictor 2 for 8-bit samples) applied to a filter's decoded output. Data is
 * split into fixed-length rows; each row is reconstructed from the previous one.
 */

#include "filters/CosPredictorFilter.h"

#include "common/Assert.h"

#include <libcos/common/CosError.h>
#include <libcos/io/CosStream.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

enum {
    COS_PREDICTOR_TIFF = 2,
    COS_PREDICTOR_PNG_MIN = 10,
    COS_PREDICTOR_PNG_MAX = 15,
};

/*
 * An upper bound on a decoded row, in bytes.
 *
 * The row geometry comes straight from /DecodeParms, so /Colors, /Columns and
 * /BitsPerComponent are all attacker-controlled. The overflow checks below stop
 * the arithmetic wrapping, but on a 64-bit host they still admit combinations
 * whose product is merely enormous: /Colors 2147483647 with /Columns 4 asks for
 * an 8.6 GB row, and two of them are allocated. A row is one scanline of image
 * data or one xref-stream entry group -- a very large real one runs to a few
 * hundred kilobytes -- so this leaves several orders of magnitude of headroom
 * while keeping the allocation bounded.
 */
#define COS_PREDICTOR_MAX_ROW_LENGTH ((size_t)(64 * 1024 * 1024))

// PNG per-row filter-type tags (RFC 2083 section 6).
enum {
    COS_PNG_NONE = 0,
    COS_PNG_SUB = 1,
    COS_PNG_UP = 2,
    COS_PNG_AVERAGE = 3,
    COS_PNG_PAETH = 4,
};

struct CosPredictorFilterContext {
    bool png;       // Whether a PNG predictor (vs. TIFF) is in effect.
    size_t colors;  // Components per sample (the TIFF differencing stride).
    size_t bpp;     // Bytes per pixel (the PNG left-sample distance), >= 1.
    size_t row_len; // Bytes per row.

    unsigned char *cur_row;
    unsigned char *prev_row;
    size_t row_pos; // Bytes of cur_row already emitted.
    bool have_row;  // Whether cur_row holds a reconstructed row.
    bool eod;       // Whether the source has been exhausted.
};

// Private function prototypes

static bool
cos_predictor_filter_init_(CosPredictorFilter *predictor_filter,
                           bool png,
                           size_t colors,
                           size_t bpp,
                           size_t row_len);

static void
cos_predictor_filter_close_(CosFilter *filter);

static size_t
cos_predictor_fill_decode_buffer_(CosFilter *filter,
                                  CosError * COS_Nullable error);

static bool
cos_predictor_load_row_(CosPredictorFilter *predictor_filter,
                        CosError * COS_Nullable error);

static bool
cos_predictor_png_unfilter_(CosPredictorFilterContext *context,
                            unsigned char tag,
                            CosError * COS_Nullable error);

static void
cos_predictor_tiff_unpredict_(CosPredictorFilterContext *context);

static unsigned char
cos_predictor_paeth_(unsigned int a, unsigned int b, unsigned int c);

// Public function implementations

CosPredictorFilter *
cos_predictor_filter_create_(int predictor,
                             size_t colors,
                             size_t bits_per_component,
                             size_t columns,
                             CosError * COS_Nullable out_error)
{
    bool png;
    if (predictor >= COS_PREDICTOR_PNG_MIN && predictor <= COS_PREDICTOR_PNG_MAX) {
        png = true;
    }
    else if (predictor == COS_PREDICTOR_TIFF) {
        png = false;
    }
    else {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_NOT_IMPLEMENTED,
                                           "Unsupported predictor"),
                            out_error);
        return NULL;
    }

    if (colors == 0 || bits_per_component == 0 || columns == 0) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_SYNTAX,
                                           "Invalid predictor parameters"),
                            out_error);
        return NULL;
    }

    if (!png && bits_per_component != 8) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_NOT_IMPLEMENTED,
                                           "TIFF predictor requires 8-bit samples"),
                            out_error);
        return NULL;
    }

    // /BitsPerComponent is restricted to this set by the specification.
    if (bits_per_component != 1 && bits_per_component != 2 &&
        bits_per_component != 4 && bits_per_component != 8 &&
        bits_per_component != 16) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_SYNTAX,
                                           "Invalid /BitsPerComponent value"),
                            out_error);
        return NULL;
    }

    // Overflow-checked row geometry.
    if (bits_per_component > SIZE_MAX / colors) {
        goto overflow;
    }
    const size_t bits_per_pixel = colors * bits_per_component;
    if (columns > SIZE_MAX / bits_per_pixel) {
        goto overflow;
    }
    const size_t row_bits = bits_per_pixel * columns;
    const size_t row_len = (row_bits / 8) + ((row_bits % 8) != 0 ? 1 : 0);

    // Bound the allocation, not just the arithmetic. See the note on the limit.
    if (row_len > COS_PREDICTOR_MAX_ROW_LENGTH) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_SYNTAX,
                                           "Predictor row length exceeds the limit"),
                            out_error);
        return NULL;
    }
    size_t bpp = (bits_per_pixel / 8) + ((bits_per_pixel % 8) != 0 ? 1 : 0);
    if (bpp == 0) {
        bpp = 1;
    }

    CosPredictorFilter * const predictor_filter = calloc(1, sizeof(CosPredictorFilter));
    if (COS_UNLIKELY(!predictor_filter)) {
        goto memory_failure;
    }

    if (COS_UNLIKELY(!cos_predictor_filter_init_(predictor_filter, png, colors, bpp, row_len))) {
        free(predictor_filter);
        goto memory_failure;
    }

    return predictor_filter;

overflow:
    COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_SYNTAX,
                                       "Predictor row geometry overflows"),
                        out_error);
    return NULL;

memory_failure:
    COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                       "Failed to allocate predictor filter"),
                        out_error);
    return NULL;
}

// Private function implementations

static bool
cos_predictor_filter_init_(CosPredictorFilter *predictor_filter,
                           bool png,
                           size_t colors,
                           size_t bpp,
                           size_t row_len)
{
    COS_IMPL_PARAM_CHECK(predictor_filter != NULL);

    static const CosFilterFunctions predictor_filter_functions_ = {
        .decode_func = &cos_predictor_fill_decode_buffer_,
        .encode_func = NULL,
        .close_func = &cos_predictor_filter_close_,
    };

    cos_filter_init(&(predictor_filter->base),
                    &predictor_filter_functions_);

    CosPredictorFilterContext * const context = calloc(1, sizeof(CosPredictorFilterContext));
    if (COS_UNLIKELY(!context)) {
        return false;
    }

    context->png = png;
    context->colors = colors;
    context->bpp = bpp;
    context->row_len = row_len;
    context->cur_row = calloc(1, row_len);
    context->prev_row = calloc(1, row_len);
    if (COS_UNLIKELY(!context->cur_row || !context->prev_row)) {
        free(context->cur_row);
        free(context->prev_row);
        free(context);
        return false;
    }

    predictor_filter->context = context;

    return true;
}

static void
cos_predictor_filter_close_(CosFilter *filter)
{
    COS_IMPL_PARAM_CHECK(filter != NULL);

    CosPredictorFilter * const predictor_filter = (CosPredictorFilter *)filter;
    if (predictor_filter->context) {
        free(predictor_filter->context->cur_row);
        free(predictor_filter->context->prev_row);
        free(predictor_filter->context);
        predictor_filter->context = NULL;
    }
}

static size_t
cos_predictor_fill_decode_buffer_(CosFilter *filter,
                                  CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(filter != NULL);

    CosPredictorFilter * const predictor_filter = (CosPredictorFilter *)filter;
    CosPredictorFilterContext * const context = predictor_filter->context;

    if (COS_UNLIKELY(!filter->source)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "No source stream"),
                            error);
        return 0;
    }

    CosFilterBuffer * const buf = &filter->buffer;

    while (buf->length < COS_FILTER_BUFFER_SIZE && !context->eod) {
        if (!context->have_row || context->row_pos >= context->row_len) {
            if (!cos_predictor_load_row_(predictor_filter, error)) {
                break;
            }
        }

        const size_t available = context->row_len - context->row_pos;
        const size_t space = COS_FILTER_BUFFER_SIZE - buf->length;
        const size_t count = available < space ? available : space;
        memcpy(buf->data + buf->length,
               context->cur_row + context->row_pos,
               count);
        buf->length += count;
        context->row_pos += count;
    }

    if (context->eod) {
        buf->eod = true;
    }

    return buf->length;
}

// Reads and reconstructs the next row into cur_row. Returns false on end-of-data
// (context->eod set, no error) or on a decode error (context->eod set, error set).
static bool
cos_predictor_load_row_(CosPredictorFilter *predictor_filter,
                        CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(predictor_filter != NULL);

    CosPredictorFilterContext * const context = predictor_filter->context;
    CosStream * const source = predictor_filter->base.source;

    // The row just finished becomes the reference for the next reconstruction.
    if (context->have_row) {
        unsigned char * const tmp = context->prev_row;
        context->prev_row = context->cur_row;
        context->cur_row = tmp;
    }

    if (context->png) {
        unsigned char tag = 0;
        if (cos_stream_read(source, &tag, 1, error) == 0) {
            context->eod = true;
            return false;
        }
        if (cos_stream_read(source, context->cur_row, context->row_len, error) != context->row_len) {
            COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_SYNTAX,
                                               "Truncated predictor row"),
                                error);
            context->eod = true;
            return false;
        }
        if (!cos_predictor_png_unfilter_(context, tag, error)) {
            context->eod = true;
            return false;
        }
    }
    else {
        const size_t got = cos_stream_read(source, context->cur_row, context->row_len, error);
        if (got == 0) {
            context->eod = true;
            return false;
        }
        if (got != context->row_len) {
            COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_SYNTAX,
                                               "Truncated predictor row"),
                                error);
            context->eod = true;
            return false;
        }
        cos_predictor_tiff_unpredict_(context);
    }

    context->have_row = true;
    context->row_pos = 0;
    return true;
}

static bool
cos_predictor_png_unfilter_(CosPredictorFilterContext *context,
                            unsigned char tag,
                            CosError * COS_Nullable error)
{
    unsigned char * const cur = context->cur_row;
    const unsigned char * const prev = context->prev_row;
    const size_t len = context->row_len;
    const size_t bpp = context->bpp;

    switch (tag) {
        case COS_PNG_NONE:
            break;

        case COS_PNG_SUB:
            for (size_t i = bpp; i < len; i++) {
                cur[i] = (unsigned char)(cur[i] + cur[i - bpp]);
            }
            break;

        case COS_PNG_UP:
            for (size_t i = 0; i < len; i++) {
                cur[i] = (unsigned char)(cur[i] + prev[i]);
            }
            break;

        case COS_PNG_AVERAGE:
            for (size_t i = 0; i < len; i++) {
                const unsigned int left = (i >= bpp) ? cur[i - bpp] : 0u;
                const unsigned int up = prev[i];
                cur[i] = (unsigned char)(cur[i] + (unsigned char)((left + up) >> 1));
            }
            break;

        case COS_PNG_PAETH:
            for (size_t i = 0; i < len; i++) {
                const unsigned int left = (i >= bpp) ? cur[i - bpp] : 0u;
                const unsigned int up = prev[i];
                const unsigned int up_left = (i >= bpp) ? prev[i - bpp] : 0u;
                cur[i] = (unsigned char)(cur[i] + cos_predictor_paeth_(left, up, up_left));
            }
            break;

        default:
            COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_SYNTAX,
                                               "Invalid PNG predictor tag"),
                                error);
            return false;
    }

    return true;
}

static void
cos_predictor_tiff_unpredict_(CosPredictorFilterContext *context)
{
    unsigned char * const cur = context->cur_row;
    const size_t len = context->row_len;
    const size_t colors = context->colors;

    for (size_t i = colors; i < len; i++) {
        cur[i] = (unsigned char)(cur[i] + cur[i - colors]);
    }
}

// The PNG Paeth predictor (RFC 2083 section 6.6).
static unsigned char
cos_predictor_paeth_(unsigned int a, unsigned int b, unsigned int c)
{
    const int p = (int)a + (int)b - (int)c;

    int pa = p - (int)a;
    int pb = p - (int)b;
    int pc = p - (int)c;
    pa = (pa < 0) ? -pa : pa;
    pb = (pb < 0) ? -pb : pb;
    pc = (pc < 0) ? -pc : pc;

    if (pa <= pb && pa <= pc) {
        return (unsigned char)a;
    }
    if (pb <= pc) {
        return (unsigned char)b;
    }
    return (unsigned char)c;
}

COS_ASSUME_NONNULL_END
