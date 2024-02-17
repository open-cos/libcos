//
// Created by david on 02/12/23.
//

#ifndef LIBCOS_IO_COS_INPUT_STREAM_READER_H
#define LIBCOS_IO_COS_INPUT_STREAM_READER_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>
#include <libcos/io/CosInputStream.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

CosInputStreamReader * COS_Nullable
cos_input_stream_reader_alloc(CosInputStream *input_stream)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_input_stream_reader_free(CosInputStreamReader *input_stream_reader);

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
