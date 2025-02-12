/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_IO_COS_FILE_STREAM_H
#define LIBCOS_IO_COS_FILE_STREAM_H

#include <libcos/common/CosDefines.h>
#include <libcos/io/CosStream.h>

#include <stdio.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef struct CosFileStreamContext {
    /**
     * The file pointer.
     */
    FILE *file;

    /**
     * Whether the file pointer is owned by the stream.
     */
    bool file_owner;
} CosFileStreamContext;

CosStream * COS_Nullable
cos_file_stream_create(const char *path,
                       const char *mode)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_stream_close);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_IO_COS_FILE_STREAM_H */
