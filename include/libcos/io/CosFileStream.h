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

typedef struct CosFileStream {
    CosStream base;

    /**
     * The file pointer.
     */
    FILE *file;

    /**
     * Whether the file pointer is owned by the stream.
     */
    bool file_owner;
} CosFileStream;

CosStream * COS_Nullable
cos_file_stream_create(const char *path,
                       const char *mode)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_stream_close);

/**
 * @brief Initializes a file stream.
 *
 * @param file_stream The file stream.
 * @param file The file pointer.
 * @param file_owner Whether the file pointer is owned by the stream.
 */
void
cos_file_stream_init(CosFileStream *file_stream,
                     FILE *file,
                     bool file_owner);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_IO_COS_FILE_STREAM_H */
