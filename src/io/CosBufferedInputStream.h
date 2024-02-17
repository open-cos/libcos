//
// Created by david on 19/12/23.
//

#ifndef LIBCOS_COS_BUFFERED_INPUT_STREAM_H
#define LIBCOS_COS_BUFFERED_INPUT_STREAM_H

#include <libcos/common/CosDefines.h>
#include <libcos/io/CosInputStream.h>

#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

CosBufferedInputStream * COS_Nullable
cos_buffered_input_stream_alloc(CosInputStream *base_stream,
                                size_t buffer_size);

void
cos_buffered_input_stream_init(CosBufferedInputStream *self,
                               CosInputStream *base_stream,
                               size_t buffer_size);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_BUFFERED_INPUT_STREAM_H */
