/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/io/CosFileStream.h"

#include "common/Assert.h"

#include <libcos/common/CosError.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

static size_t
cos_file_stream_read_(void *context,
                      void *buffer,
                      size_t size,
                      CosError * COS_Nullable out_error);

static size_t
cos_file_stream_write_(void *context,
                       const void *buffer,
                       size_t size,
                       CosError * COS_Nullable out_error);

static bool
cos_file_stream_seek_(void *context,
                      CosStreamOffset offset,
                      CosStreamOffsetWhence whence,
                      CosError * COS_Nullable out_error);

static CosStreamOffset
cos_file_stream_tell_(void *context,
                      CosError * COS_Nullable out_error);

static void
cos_file_stream_close_(void *context);

CosStream *
cos_file_stream_create(const char *path,
                       const char *mode)
{
    COS_PARAMETER_ASSERT(path != NULL);
    COS_PARAMETER_ASSERT(mode != NULL);
    if (COS_UNLIKELY(!path || !mode)) {
        return NULL;
    }

    FILE * const file = fopen(path, mode);
    if (COS_UNLIKELY(!file)) {
        return NULL;
    }

    CosStreamFunctions functions = {
        .read_func = &cos_file_stream_read_,
        .write_func = &cos_file_stream_write_,
        .seek_func = &cos_file_stream_seek_,
        .tell_func = &cos_file_stream_tell_,
        .close_func = &cos_file_stream_close_,
    };

    CosStream * const stream = cos_stream_create(&functions,
                                                 file);
    if (COS_UNLIKELY(!stream)) {
        goto failure;
    }

    return stream;

failure:
    if (file) {
        (void)fclose(file);
    }
    return NULL;
}

static size_t
cos_file_stream_read_(void *context,
                      void *buffer,
                      size_t size,
                      CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(context != NULL);
    COS_PARAMETER_ASSERT(buffer != NULL);
    COS_PARAMETER_ASSERT(size > 0);

    FILE * const file = context;

    const size_t fread_result = fread(buffer,
                                      1,
                                      size,
                                      file);
    if (fread_result < size) {
        const bool is_error = ferror(file) != 0;
        if (is_error) {
            COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_IO,
                                               "Error reading from file"),
                                out_error);
        }
    }

    return fread_result;
}

static size_t
cos_file_stream_write_(void *context,
                       const void *buffer,
                       size_t size,
                       CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(context != NULL);
    COS_PARAMETER_ASSERT(buffer != NULL);
    COS_PARAMETER_ASSERT(size > 0);

    FILE * const file = context;

    const size_t fwrite_result = fwrite(buffer,
                                        1,
                                        size,
                                        file);
    if (fwrite_result < size) {
        const bool is_error = ferror(file) != 0;
        if (is_error) {
            COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_IO,
                                               "Error writing to file"),
                                out_error);
        }
    }

    return fwrite_result;
}

static bool
cos_file_stream_seek_(void *context,
                      CosStreamOffset offset,
                      CosStreamOffsetWhence whence,
                      CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(context != NULL);

    FILE * const file = context;

    int file_whence = -1;
    switch (whence) {
        case CosStreamOffsetWhence_Set:
            file_whence = SEEK_SET;
            break;
        case CosStreamOffsetWhence_Current:
            file_whence = SEEK_CUR;
            break;
        case CosStreamOffsetWhence_End:
            file_whence = SEEK_END;
            break;
    }

    const int fseek_result = fseek(file,
                                   offset,
                                   file_whence);
    if (COS_UNLIKELY(fseek_result != 0)) {
        //const int fseek_errno = errno;
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_IO,
                                           "Error seeking in file"),
                            out_error);
        return false;
    }

    return true;
}

static CosStreamOffset
cos_file_stream_tell_(void *context,
                      CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(context != NULL);

    FILE * const file = context;

    const long offset = ftell(file);
    if (offset < 0) {
        //const int ftell_errno = errno;
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_IO,
                                           "Error getting file offset"),
                            out_error);
        return -1;
    }

    return offset;
}

static void
cos_file_stream_close_(void *context)
{
    COS_PARAMETER_ASSERT(context != NULL);

    FILE * const file = context;

    (void)fclose(file);
}

COS_ASSUME_NONNULL_END
