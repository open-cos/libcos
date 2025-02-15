/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_IO_COS_MEMORY_STREAM_H
#define LIBCOS_IO_COS_MEMORY_STREAM_H

#include <libcos/common/CosDefines.h>
#include <libcos/io/CosStream.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef struct CosMemoryStream {
    CosStream base;

    union {
        const unsigned char *read;
        unsigned char *write;
    } buffer;

    size_t size;
    size_t position;
    bool free_buffer;

} CosMemoryStream;

CosMemoryStream * COS_Nullable
cos_memory_stream_create(void *buffer,
                         size_t size,
                         bool free_buffer)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_stream_close);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_IO_COS_MEMORY_STREAM_H */
