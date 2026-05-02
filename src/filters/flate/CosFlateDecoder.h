/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_FILTERS_FLATE_COS_FLATE_DECODER_H
#define LIBCOS_FILTERS_FLATE_COS_FLATE_DECODER_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>

#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief An incremental zlib-format (RFC 1950) inflate decoder.
 *
 * This is the swappable flate-provider interface. Exactly one implementation
 * (the builtin reference inflater or a system-zlib shim) is compiled, selected
 * by the @c COS_USE_SYSTEM_ZLIB build option. Neither @c CosFlateFilter nor
 * this header depends on which provider is active.
 *
 * The decoder is a streaming state machine: callers repeatedly supply input and
 * drain output via @c cos_flate_decoder_inflate() until it reports
 * @c CosFlateStatus_Done or @c CosFlateStatus_Error.
 */
typedef struct CosFlateDecoder CosFlateDecoder;

/**
 * @brief The outcome of a single @c cos_flate_decoder_inflate() call.
 */
typedef enum CosFlateStatus {
    /**
     * @brief All provided input was consumed without reaching the end of the
     * stream. Supply more input and call again.
     */
    CosFlateStatus_NeedInput,

    /**
     * @brief The output buffer was filled before the end of the stream was
     * reached. Drain the output and call again (input may remain unconsumed).
     */
    CosFlateStatus_HasOutput,

    /**
     * @brief The end of the zlib stream was reached. No more output will be
     * produced.
     */
    CosFlateStatus_Done,

    /**
     * @brief The input was malformed. @c out_error is set when provided.
     */
    CosFlateStatus_Error,
} CosFlateStatus;

/**
 * @brief Creates a new flate decoder.
 *
 * @return The new decoder, or @c NULL if memory allocation failed.
 */
CosFlateDecoder * COS_Nullable
cos_flate_decoder_create(void);

/**
 * @brief Destroys a flate decoder.
 *
 * @param decoder The decoder to destroy.
 */
void
cos_flate_decoder_destroy(CosFlateDecoder *decoder);

/**
 * @brief Inflates one chunk, consuming input and producing output.
 *
 * @param decoder The decoder.
 * @param in The compressed input bytes.
 * @param in_len On entry, the number of bytes available at @p in; on return,
 * the number of bytes consumed.
 * @param out The output buffer for decompressed bytes.
 * @param out_len On entry, the capacity of @p out; on return, the number of
 * bytes written.
 * @param out_error On failure, set to describe the error.
 *
 * @return The status of the call.
 */
CosFlateStatus
cos_flate_decoder_inflate(CosFlateDecoder *decoder,
                          const unsigned char *in,
                          size_t *in_len,
                          unsigned char *out,
                          size_t *out_len,
                          CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_READ_ONLY(2)
    COS_ATTR_ACCESS_READ_WRITE(3)
    COS_ATTR_ACCESS_WRITE_ONLY(4)
    COS_ATTR_ACCESS_READ_WRITE(5);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_FILTERS_FLATE_COS_FLATE_DECODER_H */
