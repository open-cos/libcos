/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_IO_COS_INPUT_STREAM_READER_H
#define LIBCOS_IO_COS_INPUT_STREAM_READER_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

void
cos_input_stream_reader_destroy(CosInputStreamReader *input_stream_reader)
    COS_DEALLOCATOR_FUNC;

CosInputStreamReader * COS_Nullable
cos_input_stream_reader_create(CosStream *input_stream)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_input_stream_reader_destroy);

/**
 * @brief Reset the input stream reader.
 *
 * This function clears the input stream reader's internal buffer and resets its position to
 * the current position of the input stream.
 *
 * @param input_stream_reader The input stream reader.
 */
void
cos_input_stream_reader_reset(CosInputStreamReader *input_stream_reader);

bool
cos_input_stream_reader_is_at_end(CosInputStreamReader *input_stream_reader);

int
cos_input_stream_reader_getc(CosInputStreamReader *input_stream_reader);

int
cos_input_stream_reader_peek(CosInputStreamReader *input_stream_reader);

bool
cos_input_stream_reader_ungetc(CosInputStreamReader *input_stream_reader);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_IO_COS_INPUT_STREAM_READER_H */
