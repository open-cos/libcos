/*
 * Copyright (c) 2025 OpenCOS.
 *
 * A from-scratch, streaming zlib-format (RFC 1950 / RFC 1951) inflate decoder.
 *
 * This is a reference implementation: it favours clarity over raw speed. It is
 * a resumable state machine -- each call consumes as much of the supplied input
 * as it can and produces as much output as fits, suspending at symbol
 * granularity when either runs out. Persistent decode state (bit buffer, block
 * state, Huffman tables, 32 KiB sliding window, running Adler-32) lives in the
 * decoder so a suspended stream resumes exactly where it left off.
 */

#include "filters/flate/CosFlateDecoder.h"

#include "common/Assert.h"
#include "common/CosError.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

enum {
    COS_FLATE_MAX_BITS = 15,    // Maximum bits in a Huffman code.
    COS_FLATE_MAX_LIT = 288,    // Maximum number of literal/length codes.
    COS_FLATE_MAX_DIST = 30,    // Maximum number of distance codes.
    COS_FLATE_MAX_CODELEN = 19, // Number of code-length codes.
    COS_FLATE_WINDOW_SIZE = 32768,
    COS_FLATE_WINDOW_MASK = 32767,
    COS_FLATE_ADLER_MOD = 65521,
};

/**
 * @brief A canonical Huffman code table (counts + sorted symbols).
 */
typedef struct CosHuffman {
    int count[COS_FLATE_MAX_BITS + 1];
    int symbol[COS_FLATE_MAX_LIT];
} CosHuffman;

/**
 * @brief The decoder's resumable state machine phases.
 */
typedef enum CosFlateState {
    CosFlateState_ZlibHeader,
    CosFlateState_BlockHeader,
    CosFlateState_StoredLen,
    CosFlateState_StoredNlen,
    CosFlateState_StoredCopy,
    CosFlateState_DynHeader,
    CosFlateState_DynCodeLens,
    CosFlateState_DynLengths,
    CosFlateState_LitLen,
    CosFlateState_LenExtra,
    CosFlateState_Dist,
    CosFlateState_DistExtra,
    CosFlateState_Copy,
    CosFlateState_Adler,
    CosFlateState_Done,
    CosFlateState_Error,
} CosFlateState;

struct CosFlateDecoder {
    CosFlateState state;

    // Bit buffer: bits are consumed LSB-first from the low end of `bitbuf`.
    uint64_t bitbuf;
    unsigned bit_count;

    int bfinal; // Whether the current block is the final block.

    // Dynamic-header parsing.
    unsigned dyn_nlit;   // Number of literal/length codes (HLIT + 257).
    unsigned dyn_ndist;  // Number of distance codes (HDIST + 1).
    unsigned dyn_nclen;  // Number of code-length codes (HCLEN + 4).
    unsigned dyn_index;  // Progress cursor while filling code-length arrays.
    int dyn_pending_sym; // Decoded-but-not-yet-applied code-length symbol.

    unsigned char cl_lengths[COS_FLATE_MAX_CODELEN];
    unsigned char lengths[COS_FLATE_MAX_LIT + COS_FLATE_MAX_DIST];

    CosHuffman lit;
    CosHuffman dist;
    CosHuffman cl;

    // Pending match / stored-copy bookkeeping.
    unsigned length;     // Match length being assembled.
    unsigned distance;   // Match distance being assembled.
    unsigned extra_bits; // Number of extra bits pending for length/distance.
    unsigned copy_remaining;
    unsigned copy_dist;
    unsigned stored_remaining;

    // Byte-oriented field accumulation (Adler-32 trailer).
    unsigned io_index;
    uint32_t io_acc;

    // Sliding window (also the output history for back-references).
    unsigned char window[COS_FLATE_WINDOW_SIZE];
    unsigned window_pos;
    uint64_t out_count;

    // Running Adler-32 of the decompressed output.
    uint32_t adler_a;
    uint32_t adler_b;
};

// clang-format off

// The DEFLATE code-length code lengths appear in this permuted order.
static const unsigned char cos_flate_codelen_order_[COS_FLATE_MAX_CODELEN] = {
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15,
};

static const unsigned short cos_flate_length_base_[29] = {
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51,
    59, 67, 83, 99, 115, 131, 163, 195, 227, 258,
};

static const unsigned char cos_flate_length_extra_[29] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4,
    4, 5, 5, 5, 5, 0,
};

static const unsigned short cos_flate_dist_base_[COS_FLATE_MAX_DIST] = {
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385,
    513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577,
};

static const unsigned char cos_flate_dist_extra_[COS_FLATE_MAX_DIST] = {
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10,
    10, 11, 11, 12, 12, 13, 13,
};

// clang-format on

// Private function prototypes

static bool
cos_flate_take_bits_(CosFlateDecoder *decoder,
                     const unsigned char **in_ptr,
                     const unsigned char *in_end,
                     unsigned bit_count,
                     uint32_t *out_value);

static int
cos_flate_decode_symbol_(CosFlateDecoder *decoder,
                         const unsigned char **in_ptr,
                         const unsigned char *in_end,
                         const CosHuffman *table);

static int
cos_flate_construct_(CosHuffman *table,
                     const unsigned char *lengths,
                     int count);

static void
cos_flate_build_fixed_(CosFlateDecoder *decoder);

// Public function implementations

CosFlateDecoder *
cos_flate_decoder_create(void)
{
    CosFlateDecoder * const decoder = calloc(1, sizeof(CosFlateDecoder));
    if (COS_UNLIKELY(!decoder)) {
        return NULL;
    }

    decoder->state = CosFlateState_ZlibHeader;
    decoder->dyn_pending_sym = -1;
    decoder->adler_a = 1;
    decoder->adler_b = 0;

    return decoder;
}

void
cos_flate_decoder_destroy(CosFlateDecoder *decoder)
{
    COS_IMPL_PARAM_CHECK(decoder != NULL);

    free(decoder);
}

CosFlateStatus
cos_flate_decoder_inflate(CosFlateDecoder *decoder,
                          const unsigned char *in,
                          size_t *in_len,
                          unsigned char *out,
                          size_t *out_len,
                          CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(decoder != NULL);
    COS_IMPL_PARAM_CHECK(in != NULL);
    COS_IMPL_PARAM_CHECK(in_len != NULL);
    COS_IMPL_PARAM_CHECK(out != NULL);
    COS_IMPL_PARAM_CHECK(out_len != NULL);

    const unsigned char *in_ptr = in;
    const unsigned char * const in_end = in + *in_len;
    unsigned char *out_ptr = out;
    unsigned char * const out_end = out + *out_len;

    CosFlateStatus result;
    const char *error_message = NULL;

#define COS_FLATE_PUT_BYTE(byte_value)                                                      \
    do {                                                                                    \
        const unsigned char b_ = (unsigned char)(byte_value);                               \
        decoder->window[decoder->window_pos] = b_;                                          \
        decoder->window_pos = (decoder->window_pos + 1u) & (unsigned)COS_FLATE_WINDOW_MASK; \
        decoder->out_count += 1;                                                            \
        decoder->adler_a += (uint32_t)b_;                                                   \
        if (decoder->adler_a >= COS_FLATE_ADLER_MOD) {                                      \
            decoder->adler_a -= COS_FLATE_ADLER_MOD;                                        \
        }                                                                                   \
        decoder->adler_b += decoder->adler_a;                                               \
        if (decoder->adler_b >= COS_FLATE_ADLER_MOD) {                                      \
            decoder->adler_b -= COS_FLATE_ADLER_MOD;                                        \
        }                                                                                   \
        *out_ptr++ = b_;                                                                    \
    } while (0)

    for (;;) {
        switch (decoder->state) {
            case CosFlateState_ZlibHeader: {
                uint32_t header;
                if (!cos_flate_take_bits_(decoder, &in_ptr, in_end, 16, &header)) {
                    result = CosFlateStatus_NeedInput;
                    goto suspend;
                }
                const uint32_t cmf = header & 0xffu;
                const uint32_t flg = (header >> 8) & 0xffu;
                if ((cmf & 0x0fu) != 8 ||            // Compression method must be DEFLATE.
                    ((cmf << 8) | flg) % 31u != 0 || // Header checksum.
                    (flg & 0x20u) != 0) {            // Preset dictionaries are unsupported.
                    error_message = "Invalid zlib header";
                    goto fail;
                }
                decoder->state = CosFlateState_BlockHeader;
                break;
            }

            case CosFlateState_BlockHeader: {
                uint32_t header;
                if (!cos_flate_take_bits_(decoder, &in_ptr, in_end, 3, &header)) {
                    result = CosFlateStatus_NeedInput;
                    goto suspend;
                }
                decoder->bfinal = (int)(header & 1u);
                const uint32_t btype = (header >> 1) & 3u;
                if (btype == 0) {
                    // Stored block: discard bits back to a byte boundary.
                    const unsigned drop = decoder->bit_count & 7u;
                    decoder->bitbuf >>= drop;
                    decoder->bit_count -= drop;
                    decoder->state = CosFlateState_StoredLen;
                }
                else if (btype == 1) {
                    cos_flate_build_fixed_(decoder);
                    decoder->state = CosFlateState_LitLen;
                }
                else if (btype == 2) {
                    decoder->state = CosFlateState_DynHeader;
                }
                else {
                    error_message = "Invalid block type";
                    goto fail;
                }
                break;
            }

            case CosFlateState_StoredLen: {
                uint32_t len;
                if (!cos_flate_take_bits_(decoder, &in_ptr, in_end, 16, &len)) {
                    result = CosFlateStatus_NeedInput;
                    goto suspend;
                }
                decoder->stored_remaining = len;
                decoder->io_acc = len;
                decoder->state = CosFlateState_StoredNlen;
                break;
            }

            case CosFlateState_StoredNlen: {
                uint32_t nlen;
                if (!cos_flate_take_bits_(decoder, &in_ptr, in_end, 16, &nlen)) {
                    result = CosFlateStatus_NeedInput;
                    goto suspend;
                }
                if ((nlen & 0xffffu) != ((~decoder->io_acc) & 0xffffu)) {
                    error_message = "Corrupt stored block length";
                    goto fail;
                }
                decoder->state = CosFlateState_StoredCopy;
                break;
            }

            case CosFlateState_StoredCopy: {
                while (decoder->stored_remaining > 0) {
                    if (out_ptr == out_end) {
                        result = CosFlateStatus_HasOutput;
                        goto suspend;
                    }
                    uint32_t byte_value;
                    if (!cos_flate_take_bits_(decoder, &in_ptr, in_end, 8, &byte_value)) {
                        result = CosFlateStatus_NeedInput;
                        goto suspend;
                    }
                    COS_FLATE_PUT_BYTE(byte_value);
                    decoder->stored_remaining -= 1;
                }
                decoder->state = decoder->bfinal ? CosFlateState_Adler
                                                 : CosFlateState_BlockHeader;
                break;
            }

            case CosFlateState_DynHeader: {
                uint32_t header;
                if (!cos_flate_take_bits_(decoder, &in_ptr, in_end, 14, &header)) {
                    result = CosFlateStatus_NeedInput;
                    goto suspend;
                }
                decoder->dyn_nlit = (header & 0x1fu) + 257u;
                decoder->dyn_ndist = ((header >> 5) & 0x1fu) + 1u;
                decoder->dyn_nclen = ((header >> 10) & 0x0fu) + 4u;
                if (decoder->dyn_nlit > COS_FLATE_MAX_LIT ||
                    decoder->dyn_ndist > COS_FLATE_MAX_DIST) {
                    error_message = "Too many dynamic codes";
                    goto fail;
                }
                for (unsigned i = 0; i < COS_FLATE_MAX_CODELEN; i++) {
                    decoder->cl_lengths[i] = 0;
                }
                decoder->dyn_index = 0;
                decoder->state = CosFlateState_DynCodeLens;
                break;
            }

            case CosFlateState_DynCodeLens: {
                while (decoder->dyn_index < decoder->dyn_nclen) {
                    uint32_t value;
                    if (!cos_flate_take_bits_(decoder, &in_ptr, in_end, 3, &value)) {
                        result = CosFlateStatus_NeedInput;
                        goto suspend;
                    }
                    decoder->cl_lengths[cos_flate_codelen_order_[decoder->dyn_index]] =
                        (unsigned char)value;
                    decoder->dyn_index += 1;
                }
                if (cos_flate_construct_(&decoder->cl,
                                         decoder->cl_lengths,
                                         COS_FLATE_MAX_CODELEN) < 0) {
                    error_message = "Invalid code-length code";
                    goto fail;
                }
                decoder->dyn_index = 0;
                decoder->dyn_pending_sym = -1;
                decoder->state = CosFlateState_DynLengths;
                break;
            }

            case CosFlateState_DynLengths: {
                const unsigned total = decoder->dyn_nlit + decoder->dyn_ndist;
                while (decoder->dyn_index < total) {
                    if (decoder->dyn_pending_sym < 0) {
                        const int sym = cos_flate_decode_symbol_(decoder, &in_ptr, in_end, &decoder->cl);
                        if (sym == -1) {
                            result = CosFlateStatus_NeedInput;
                            goto suspend;
                        }
                        if (sym < 0) {
                            error_message = "Invalid code-length symbol";
                            goto fail;
                        }
                        decoder->dyn_pending_sym = sym;
                    }

                    const int sym = decoder->dyn_pending_sym;
                    if (sym < 16) {
                        decoder->lengths[decoder->dyn_index++] = (unsigned char)sym;
                        decoder->dyn_pending_sym = -1;
                    }
                    else {
                        unsigned extra_count;
                        unsigned base;
                        unsigned char repeat_value;
                        if (sym == 16) {
                            if (decoder->dyn_index == 0) {
                                error_message = "Invalid repeat code";
                                goto fail;
                            }
                            extra_count = 2;
                            base = 3;
                            repeat_value = decoder->lengths[decoder->dyn_index - 1];
                        }
                        else if (sym == 17) {
                            extra_count = 3;
                            base = 3;
                            repeat_value = 0;
                        }
                        else {
                            extra_count = 7;
                            base = 11;
                            repeat_value = 0;
                        }

                        uint32_t extra;
                        if (!cos_flate_take_bits_(decoder, &in_ptr, in_end, extra_count, &extra)) {
                            result = CosFlateStatus_NeedInput;
                            goto suspend;
                        }
                        const unsigned repeat = base + extra;
                        if (decoder->dyn_index + repeat > total) {
                            error_message = "Repeat overruns code lengths";
                            goto fail;
                        }
                        for (unsigned i = 0; i < repeat; i++) {
                            decoder->lengths[decoder->dyn_index++] = repeat_value;
                        }
                        decoder->dyn_pending_sym = -1;
                    }
                }

                if (cos_flate_construct_(&decoder->lit,
                                         decoder->lengths,
                                         (int)decoder->dyn_nlit) < 0) {
                    error_message = "Invalid literal/length code";
                    goto fail;
                }
                const int dist_err =
                    cos_flate_construct_(&decoder->dist,
                                         decoder->lengths + decoder->dyn_nlit,
                                         (int)decoder->dyn_ndist);
                // An incomplete distance code is only valid with a single code.
                if (dist_err < 0 ||
                    (dist_err > 0 &&
                     ((int)decoder->dyn_ndist - decoder->dist.count[0] != 1))) {
                    error_message = "Invalid distance code";
                    goto fail;
                }
                decoder->state = CosFlateState_LitLen;
                break;
            }

            case CosFlateState_LitLen: {
                if (out_ptr == out_end) {
                    result = CosFlateStatus_HasOutput;
                    goto suspend;
                }
                const int sym = cos_flate_decode_symbol_(decoder, &in_ptr, in_end, &decoder->lit);
                if (sym == -1) {
                    result = CosFlateStatus_NeedInput;
                    goto suspend;
                }
                if (sym < 0) {
                    error_message = "Invalid literal/length symbol";
                    goto fail;
                }
                if (sym < 256) {
                    COS_FLATE_PUT_BYTE(sym);
                }
                else if (sym == 256) {
                    decoder->state = decoder->bfinal ? CosFlateState_Adler
                                                     : CosFlateState_BlockHeader;
                }
                else if (sym <= 285) {
                    const unsigned index = (unsigned)sym - 257u;
                    decoder->length = cos_flate_length_base_[index];
                    decoder->extra_bits = cos_flate_length_extra_[index];
                    decoder->state = CosFlateState_LenExtra;
                }
                else {
                    error_message = "Invalid length symbol";
                    goto fail;
                }
                break;
            }

            case CosFlateState_LenExtra: {
                uint32_t extra;
                if (!cos_flate_take_bits_(decoder, &in_ptr, in_end, decoder->extra_bits, &extra)) {
                    result = CosFlateStatus_NeedInput;
                    goto suspend;
                }
                decoder->length += extra;
                decoder->state = CosFlateState_Dist;
                break;
            }

            case CosFlateState_Dist: {
                const int sym = cos_flate_decode_symbol_(decoder, &in_ptr, in_end, &decoder->dist);
                if (sym == -1) {
                    result = CosFlateStatus_NeedInput;
                    goto suspend;
                }
                if (sym < 0 || sym >= COS_FLATE_MAX_DIST) {
                    error_message = "Invalid distance symbol";
                    goto fail;
                }
                decoder->distance = cos_flate_dist_base_[sym];
                decoder->extra_bits = cos_flate_dist_extra_[sym];
                decoder->state = CosFlateState_DistExtra;
                break;
            }

            case CosFlateState_DistExtra: {
                uint32_t extra;
                if (!cos_flate_take_bits_(decoder, &in_ptr, in_end, decoder->extra_bits, &extra)) {
                    result = CosFlateStatus_NeedInput;
                    goto suspend;
                }
                decoder->distance += extra;
                if (decoder->distance > decoder->out_count) {
                    error_message = "Distance too far back";
                    goto fail;
                }
                decoder->copy_remaining = decoder->length;
                decoder->copy_dist = decoder->distance;
                decoder->state = CosFlateState_Copy;
                break;
            }

            case CosFlateState_Copy: {
                while (decoder->copy_remaining > 0) {
                    if (out_ptr == out_end) {
                        result = CosFlateStatus_HasOutput;
                        goto suspend;
                    }
                    const unsigned src = (decoder->window_pos - decoder->copy_dist) &
                                         (unsigned)COS_FLATE_WINDOW_MASK;
                    COS_FLATE_PUT_BYTE(decoder->window[src]);
                    decoder->copy_remaining -= 1;
                }
                decoder->state = CosFlateState_LitLen;
                break;
            }

            case CosFlateState_Adler: {
                // Consume any partial-byte remainder to reach a byte boundary,
                // but only on first entry (io_index tracks bytes already read).
                if (decoder->io_index == 0) {
                    const unsigned drop = decoder->bit_count & 7u;
                    decoder->bitbuf >>= drop;
                    decoder->bit_count -= drop;
                    decoder->io_acc = 0;
                }
                while (decoder->io_index < 4) {
                    uint32_t byte_value;
                    if (!cos_flate_take_bits_(decoder, &in_ptr, in_end, 8, &byte_value)) {
                        result = CosFlateStatus_NeedInput;
                        goto suspend;
                    }
                    decoder->io_acc = (decoder->io_acc << 8) | (byte_value & 0xffu);
                    decoder->io_index += 1;
                }
                const uint32_t expected = (decoder->adler_b << 16) | decoder->adler_a;
                if (decoder->io_acc != expected) {
                    error_message = "Adler-32 checksum mismatch";
                    goto fail;
                }
                decoder->state = CosFlateState_Done;
                break;
            }

            case CosFlateState_Done:
                result = CosFlateStatus_Done;
                goto suspend;

            case CosFlateState_Error:
                error_message = "Flate stream in error state";
                goto fail;
        }
    }

fail:
    decoder->state = CosFlateState_Error;
    COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_SYNTAX,
                                       error_message != NULL ? error_message
                                                             : "Flate decode error"),
                        out_error);
    result = CosFlateStatus_Error;

suspend:
    *in_len = (size_t)(in_ptr - in);
    *out_len = (size_t)(out_ptr - out);
    return result;

#undef COS_FLATE_PUT_BYTE
}

// Private function implementations

/**
 * Consumes exactly @p bit_count bits (LSB-first) from the stream.
 *
 * Bits already buffered persist in the decoder across calls, so a @c false
 * return (input exhausted before enough bits were buffered) leaves the decoder
 * resumable: the bytes pulled into the bit buffer are retained and reported as
 * consumed.
 */
static bool
cos_flate_take_bits_(CosFlateDecoder *decoder,
                     const unsigned char **in_ptr,
                     const unsigned char *in_end,
                     unsigned bit_count,
                     uint32_t *out_value)
{
    while (decoder->bit_count < bit_count) {
        if (*in_ptr == in_end) {
            return false;
        }
        decoder->bitbuf |= (uint64_t)(**in_ptr) << decoder->bit_count;
        *in_ptr += 1;
        decoder->bit_count += 8;
    }

    *out_value = (bit_count == 0)
                     ? 0u
                     : (uint32_t)(decoder->bitbuf & (((uint64_t)1 << bit_count) - 1));
    decoder->bitbuf >>= bit_count;
    decoder->bit_count -= bit_count;
    return true;
}

/**
 * Decodes one canonical Huffman symbol, peeking bits so that an exhausted input
 * (return value -1) consumes nothing and can be retried. Returns -2 on a code
 * that is longer than the table allows.
 */
static int
cos_flate_decode_symbol_(CosFlateDecoder *decoder,
                         const unsigned char **in_ptr,
                         const unsigned char *in_end,
                         const CosHuffman *table)
{
    int code = 0;
    int first = 0;
    int index = 0;

    for (unsigned len = 1; len <= COS_FLATE_MAX_BITS; len++) {
        while (decoder->bit_count < len) {
            if (*in_ptr == in_end) {
                return -1;
            }
            decoder->bitbuf |= (uint64_t)(**in_ptr) << decoder->bit_count;
            *in_ptr += 1;
            decoder->bit_count += 8;
        }

        code |= (int)((decoder->bitbuf >> (len - 1)) & 1u);
        const int count = table->count[len];
        if (code - count < first) {
            decoder->bitbuf >>= len;
            decoder->bit_count -= len;
            return table->symbol[index + (code - first)];
        }
        index += count;
        first += count;
        first <<= 1;
        code <<= 1;
    }

    return -2;
}

/**
 * Builds a canonical Huffman table from a list of code lengths. Returns 0 on a
 * complete code, a positive value for an incomplete code, or a negative value
 * for an over-subscribed (invalid) code.
 */
static int
cos_flate_construct_(CosHuffman *table,
                     const unsigned char *lengths,
                     int count)
{
    for (int len = 0; len <= COS_FLATE_MAX_BITS; len++) {
        table->count[len] = 0;
    }
    for (int sym = 0; sym < count; sym++) {
        table->count[lengths[sym]]++;
    }
    if (table->count[0] == count) {
        return 0;
    }

    int left = 1;
    for (int len = 1; len <= COS_FLATE_MAX_BITS; len++) {
        left <<= 1;
        left -= table->count[len];
        if (left < 0) {
            return left;
        }
    }

    int offsets[COS_FLATE_MAX_BITS + 1];
    offsets[1] = 0;
    for (int len = 1; len < COS_FLATE_MAX_BITS; len++) {
        offsets[len + 1] = offsets[len] + table->count[len];
    }
    for (int sym = 0; sym < count; sym++) {
        if (lengths[sym] != 0) {
            table->symbol[offsets[lengths[sym]]++] = sym;
        }
    }

    return left;
}

/**
 * Populates the fixed literal/length and distance Huffman tables (RFC 1951
 * section 3.2.6).
 */
static void
cos_flate_build_fixed_(CosFlateDecoder *decoder)
{
    unsigned char lit_lengths[COS_FLATE_MAX_LIT];
    unsigned char dist_lengths[COS_FLATE_MAX_DIST];

    int sym = 0;
    for (; sym < 144; sym++) {
        lit_lengths[sym] = 8;
    }
    for (; sym < 256; sym++) {
        lit_lengths[sym] = 9;
    }
    for (; sym < 280; sym++) {
        lit_lengths[sym] = 7;
    }
    for (; sym < 288; sym++) {
        lit_lengths[sym] = 8;
    }
    for (int i = 0; i < COS_FLATE_MAX_DIST; i++) {
        dist_lengths[i] = 5;
    }

    (void)cos_flate_construct_(&decoder->lit, lit_lengths, COS_FLATE_MAX_LIT);
    (void)cos_flate_construct_(&decoder->dist, dist_lengths, COS_FLATE_MAX_DIST);
}

COS_ASSUME_NONNULL_END
