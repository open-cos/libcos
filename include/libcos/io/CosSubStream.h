/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_IO_COS_SUB_STREAM_H
#define LIBCOS_IO_COS_SUB_STREAM_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/io/CosStream.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * A read-only stream presenting a fixed byte window of a seekable source stream.
 *
 * The window covers @c base_offset through @c base_offset + @c length of the source. Reads and
 * seeks are expressed relative to the window; the source is seeked before each read, so a single
 * seekable source can back several sub-streams that are used sequentially.
 */
typedef struct CosSubStream {
    CosStream base;

    /**
     * The underlying source stream, borrowed unless @c source_owner is @c true.
     */
    CosStream *source;

    /**
     * Whether the source stream is owned by this sub-stream.
     */
    bool source_owner;

    /**
     * The offset of the window within the source stream.
     */
    CosStreamOffset base_offset;

    /**
     * The length of the window in bytes.
     */
    size_t length;

    /**
     * The current read position within the window (0..length).
     */
    size_t position;
} CosSubStream;

/**
 * Creates a read-only windowed view over a seekable source stream.
 *
 * @param source The source stream. Must be seekable.
 * @param base_offset The offset of the window within the source.
 * @param length The length of the window in bytes.
 * @param source_owner Whether the returned stream takes ownership of @p source (and closes it
 *                     when the sub-stream is closed).
 * @param out_error On failure, set to describe the error.
 *
 * @return A new sub-stream (close with @c cos_stream_close), or @c NULL on error.
 */
COS_API CosStream * COS_Nullable
cos_sub_stream_create(CosStream *source,
                      CosStreamOffset base_offset,
                      size_t length,
                      bool source_owner,
                      CosError * COS_Nullable out_error)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_stream_close)
    COS_ATTR_ACCESS_WRITE_ONLY(5);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_IO_COS_SUB_STREAM_H */
