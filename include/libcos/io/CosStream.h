/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_IO_COS_STREAM_H
#define LIBCOS_IO_COS_STREAM_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef signed long long CosStreamOffset;
#define COS_STREAM_OFFSET_MAX LLONG_MAX

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

typedef struct CosStreamFunctions {
    size_t (* COS_Nullable read_func)(void *context,
                                      void *buffer,
                                      size_t count,
                                      CosError * COS_Nullable out_error)
        COS_ATTR_ACCESS_WRITE_ONLY_SIZE(2, 3)
        COS_ATTR_ACCESS_WRITE_ONLY(4);

    size_t (* COS_Nullable write_func)(void *context,
                                       const void *buffer,
                                       size_t count,
                                       CosError * COS_Nullable out_error)
        COS_ATTR_ACCESS_READ_ONLY_SIZE(2, 3)
        COS_ATTR_ACCESS_WRITE_ONLY(4);

    /**
     * @brief Adjusts the stream's offset.
     *
     * @param context The context of the stream.
     * @param offset The offset adjustment.
     * @param whence The type of adjustment.
     * @param out_error The error information.
     *
     * @return @c true if the adjustment was successful, @c false otherwise.
     */
    bool (* COS_Nullable seek_func)(void *context,
                                    CosStreamOffset offset,
                                    CosStreamOffsetWhence whence,
                                    CosError * COS_Nullable out_error)
        COS_ATTR_ACCESS_WRITE_ONLY(4);

    /**
     * @brief Returns the current offset of the stream.
     *
     * @param context The context of the stream.
     * @param out_error The error information.
     *
     * @return The current offset of the stream, or @c -1 if an error occurred.
     */
    CosStreamOffset (*tell_func)(void *context,
                                 CosError * COS_Nullable out_error)
        COS_ATTR_ACCESS_WRITE_ONLY(2);

    /**
     * @brief Returns whether the stream is at the end.
     *
     * @param context The context of the stream.
     * @param out_error The error information.
     *
     * @return @c true if the stream is at the end, @c false otherwise.
     */
    bool (*eof_func)(void *context);

    /**
     * @brief Closes the stream.
     *
     * @param context The context of the stream.
     */
    void (*close_func)(void *context);

} CosStreamFunctions;

void
cos_stream_close(CosStream *stream)
    COS_DEALLOCATOR_FUNC;

CosStream * COS_Nullable
cos_stream_create(const CosStreamFunctions *functions,
                  void *context)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_stream_close);

bool
cos_stream_is_valid(const CosStream *stream);

bool
cos_stream_can_read(const CosStream *stream);

size_t
cos_stream_read(CosStream *stream,
                void *buffer,
                size_t count,
                CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY_SIZE(2, 3)
    COS_ATTR_ACCESS_WRITE_ONLY(4);

bool
cos_stream_can_write(const CosStream *stream);

size_t
cos_stream_write(CosStream *stream,
                 const void *buffer,
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
