/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_IO_COS_STREAM_H
#define LIBCOS_IO_COS_STREAM_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosBasicTypes.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief The types of a stream offset adjustment.
 */
typedef enum CosStreamOffsetWhence {
    /**
     * @brief Set the stream's offset to @p offset .
     */
    CosStreamOffsetWhence_Set = 0,

    /**
     * @brief Set the stream's offset to the current offset plus @p offset .
     */
    CosStreamOffsetWhence_Current = 1,

    /**
     * @brief Set the stream's offset to the end of the stream plus @p offset .
     */
    CosStreamOffsetWhence_End = 2,

} CosStreamOffsetWhence;

/**
 * @brief The functions implemented by a stream.
 */
typedef struct CosStreamFunctions {
    /**
     * @brief Reads data from the stream.
     *
     * @param stream The stream.
     * @param buffer The output buffer to read into.
     * @param count The maximum number of bytes to read.
     * @param out_error The error information.
     *
     * @return The number of bytes read, or @c 0 if an error occurred.
     */
    size_t (* COS_Nullable read_func)(CosStream *stream,
                                      COS_PARAM_SPEC(out, nonnull, sized_by(count)) void *buffer,
                                      size_t count,
                                      CosError * COS_Nullable out_error)
        COS_ATTR_ACCESS_WRITE_ONLY_SIZE(2, 3)
        COS_ATTR_ACCESS_WRITE_ONLY(4);

    /**
     * @brief Writes data to the stream.
     *
     * @param stream The stream.
     * @param buffer The input buffer to be written.
     * @param count The maximum number of bytes to write and the size of @p buffer .
     * @param out_error The error information.
     *
     * @return The number of bytes written, or @c 0 if an error occurred.
     */
    size_t (* COS_Nullable write_func)(CosStream *stream,
                                       COS_PARAM_SPEC(in, nonnull, sized_by(count)) const void *buffer,
                                       size_t count,
                                       CosError * COS_Nullable out_error)
        COS_ATTR_ACCESS_READ_ONLY_SIZE(2, 3)
        COS_ATTR_ACCESS_WRITE_ONLY(4);

    /**
     * @brief Adjusts the stream's offset.
     *
     * @param stream The stream.
     * @param offset The offset adjustment.
     * @param whence The type of adjustment.
     * @param out_error The error information.
     *
     * @return @c true if the adjustment was successful, @c false otherwise.
     */
    bool (* COS_Nullable seek_func)(CosStream *stream,
                                    CosStreamOffset offset,
                                    CosStreamOffsetWhence whence,
                                    CosError * COS_Nullable out_error)
        COS_ATTR_ACCESS_WRITE_ONLY(4);

    /**
     * @brief Returns the current offset of the stream.
     *
     * @param stream The stream.
     * @param out_error The error information.
     *
     * @return The current offset of the stream, or @c -1 if an error occurred.
     */
    CosStreamOffset (*tell_func)(CosStream *stream,
                                 CosError * COS_Nullable out_error)
        COS_ATTR_ACCESS_WRITE_ONLY(2);

    /**
     * @brief Returns whether the stream is at the end.
     *
     * @param stream The stream.
     * @param out_error The error information.
     *
     * @return @c true if the stream is at the end, @c false otherwise.
     */
    bool (*eof_func)(CosStream *stream);

    /**
     * @brief Closes the stream.
     *
     * @param stream The stream.
     */
    void (*close_func)(CosStream *stream);

} CosStreamFunctions;

typedef enum CosStreamFlags {
    CosStreamFlag_EOF = 1 << 0,
} CosStreamFlags;

/**
 * @brief A stream container.
 *
 * A stream is an abstraction over a sequence of data that is accessed via a set of client-provided
 * functions.
 */
struct CosStream {
    CosStreamFunctions functions;

    CosStreamFlags flags;
};

void
cos_stream_close(CosStream *stream)
    COS_DEALLOCATOR_FUNC;

CosStream * COS_Nullable
cos_stream_create(const CosStreamFunctions *functions)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_stream_close);

/**
 * @brief Initializes a stream.
 *
 * @param stream The stream to be initialized.
 * @param functions The functions to be used by the stream.
 */
void
cos_stream_init(CosStream *stream,
                const CosStreamFunctions *functions);

bool
cos_stream_is_valid(const CosStream *stream);

/**
 * @brief Returns whether the stream can read.
 *
 * @param stream The stream.
 *
 * @return @c true if the stream can read, @c false otherwise.
 */
bool
cos_stream_can_read(const CosStream *stream);

/**
 * @brief Reads data from the stream.
 *
 * @param stream The stream.
 * @param buffer The output buffer to read into.
 * @param count The maximum number of bytes to read and the size of @p buffer .
 * @param out_error The error information.
 *
 * @return The number of bytes read, or @c 0 if an error occurred.
 */
size_t
cos_stream_read(CosStream *stream,
                COS_PARAM_SPEC(out, nonnull, sized_by(count)) void *buffer,
                size_t count,
                CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY_SIZE(2, 3)
    COS_ATTR_ACCESS_WRITE_ONLY(4);

/**
 * @brief Returns whether the stream can write.
 *
 * @param stream The stream.
 *
 * @return @c true if the stream can write, @c false otherwise.
 */
bool
cos_stream_can_write(const CosStream *stream);

/**
 * @brief Writes data to the stream.
 *
 * @param stream The stream.
 * @param buffer The input buffer to be written.
 * @param count The maximum number of bytes to write and the size of @p buffer .
 * @param out_error The error information.
 *
 * @return The number of bytes written, or @c 0 if an error occurred.
 */
size_t
cos_stream_write(CosStream *stream,
                 COS_PARAM_SPEC(in, nonnull, sized_by(count)) const void *buffer,
                 size_t count,
                 CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_READ_ONLY_SIZE(2, 3)
    COS_ATTR_ACCESS_WRITE_ONLY(4);

/**
 * @brief Returns whether the stream can seek.
 *
 * @param stream The stream.
 *
 * @return @c true if the stream can seek, @c false otherwise.
 */
bool
cos_stream_can_seek(const CosStream *stream);

/**
 * @brief Adjusts the stream's offset.
 *
 * @param stream The stream.
 * @param offset The offset adjustment.
 * @param whence The type of adjustment.
 * @param out_error The error information.
 *
 * @return @c true if the adjustment was successful, @c false otherwise.
 */
bool
cos_stream_seek(CosStream *stream,
                CosStreamOffset offset,
                CosStreamOffsetWhence whence,
                CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(4);

/**
 * @brief Returns the current offset of the stream from the beginning.
 *
 * @param stream The stream.
 * @param out_error The error information.
 *
 * @return The current position of the stream, or @c -1 if an error occurred.
 */
CosStreamOffset
cos_stream_get_position(CosStream *stream,
                        CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(2);

bool
cos_stream_is_at_end(CosStream *stream,
                     CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(2);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_IO_COS_STREAM_H */
