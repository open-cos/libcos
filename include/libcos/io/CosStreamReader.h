/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_IO_COS_STREAM_READER_H
#define LIBCOS_IO_COS_STREAM_READER_H

#include <libcos/common/CosBasicTypes.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

void
cos_stream_reader_destroy(CosStreamReader *stream_reader)
    COS_DEALLOCATOR_FUNC;

CosStreamReader * COS_Nullable
cos_stream_reader_create(CosStream *input_stream)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_stream_reader_destroy);

/**
 * @brief Reset the stream reader.
 *
 * This function clears the stream reader's internal buffer and resets its position to
 * the current position of the input stream.
 *
 * @param stream_reader The stream reader.
 */
void
cos_stream_reader_reset(CosStreamReader *stream_reader);

/**
 * @brief Returns the current position of the stream reader.
 *
 * @param stream_reader The stream reader.
 *
 * @return The current position of the stream reader.
 */
CosStreamOffset
cos_stream_reader_get_position(CosStreamReader *stream_reader);

/**
 * @brief Returns whether the stream reader is at the end of the input stream.
 *
 * @param stream_reader The stream reader.
 *
 * @return @c true if the stream reader is at the end of the input stream, @c false otherwise.
 */
bool
cos_stream_reader_is_at_end(CosStreamReader *stream_reader);

/**
 * @brief Reads a character from the stream.
 *
 * This function reads a character from the stream and advances the stream reader's position.
 *
 * @param stream_reader The stream reader.
 *
 * @return The character read, or @c EOF if the end of the stream has been reached.
 */
int
cos_stream_reader_getc(CosStreamReader *stream_reader);

/**
 * @brief Peeks the next character from the stream.
 *
 * This function returns the next character from the stream without consuming it.
 *
 * @param stream_reader The stream reader.
 *
 * @return The next character from the stream, or @c EOF if the end of the stream has been reached.
 */
int
cos_stream_reader_peek(CosStreamReader *stream_reader);

/**
 * @brief Un-reads the last character read from the stream.
 *
 * This function moves the stream reader's position back by one character.
 *
 * @param stream_reader The stream reader.
 *
 * @return @c true if the operation was successful, @c false otherwise.
 */
bool
cos_stream_reader_ungetc(CosStreamReader *stream_reader);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_IO_COS_STREAM_READER_H */
