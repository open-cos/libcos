/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "config.h"

#include "common/Assert.h"

#include "libcos/io/CosFileStream.h"

#include <libcos/common/CosError.h>

#include <stdlib.h>
#include <string.h>

#if COS_HAS_LARGE_FILE_SUPPORT
    #define cos_fseek fseeko
#else
    #define cos_fseek fseek
#endif

COS_ASSUME_NONNULL_BEGIN

static bool
cos_file_stream_is_valid_(const CosFileStream *file_stream);

static size_t
cos_file_stream_read_(CosStream *stream,
                      void *buffer,
                      size_t size,
                      CosError * COS_Nullable out_error);

static size_t
cos_file_stream_write_(CosStream *stream,
                       const void *buffer,
                       size_t size,
                       CosError * COS_Nullable out_error);

static bool
cos_file_stream_seek_(CosStream *stream,
                      CosStreamOffset offset,
                      CosStreamOffsetWhence whence,
                      CosError * COS_Nullable out_error);

static CosStreamOffset
cos_file_stream_tell_(CosStream *stream,
                      CosError * COS_Nullable out_error);

static bool
cos_file_stream_eof_(CosStream *stream);

static void
cos_file_stream_close_(CosStream *stream);

CosStream *
cos_file_stream_create(const char *path,
                       const char *mode)
{
    COS_API_PARAM_CHECK(path != NULL);
    COS_API_PARAM_CHECK(mode != NULL);
    if (COS_UNLIKELY(!path || !mode)) {
        return NULL;
    }

    CosFileStream *file_stream = NULL;
    FILE *file = NULL;

    // Try opening the file first to avoid allocating memory unnecessarily.
    file = fopen(path, mode);
    if (COS_UNLIKELY(!file)) {
        goto failure;
    }

    file_stream = calloc(1, sizeof(CosFileStream));
    if (COS_UNLIKELY(!file_stream)) {
        goto failure;
    }

    cos_file_stream_init(file_stream,
                         file,
                         true);

    return (CosStream *)file_stream;

failure:
    if (file) {
        (void)fclose(file);
    }
    if (file_stream) {
        free(file_stream);
    }
    return NULL;
}

void
cos_file_stream_init(CosFileStream *file_stream,
                     FILE *file,
                     bool file_owner)
{
    COS_API_PARAM_CHECK(file_stream != NULL);
    COS_API_PARAM_CHECK(file != NULL);
    if (COS_UNLIKELY(!file_stream || !file)) {
        return;
    }

    file_stream->file = file;
    file_stream->file_owner = file_owner;

    const CosStreamFunctions functions = {
        .read_func = &cos_file_stream_read_,
        .write_func = &cos_file_stream_write_,
        .seek_func = &cos_file_stream_seek_,
        .tell_func = &cos_file_stream_tell_,
        .eof_func = &cos_file_stream_eof_,
        .close_func = &cos_file_stream_close_,
    };

    cos_stream_init(&(file_stream->base),
                    &functions);
}

static bool
cos_file_stream_is_valid_(const CosFileStream *file_stream)
{
    COS_IMPL_PARAM_CHECK(file_stream != NULL);

    return (file_stream->file != NULL);
}

static size_t
cos_file_stream_read_(CosStream *stream,
                      void *buffer,
                      size_t size,
                      CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(stream != NULL);
    COS_IMPL_PARAM_CHECK(buffer != NULL);
    COS_IMPL_PARAM_CHECK(size > 0);

    CosFileStream * const file_stream = (CosFileStream *)stream;
    COS_ASSERT(cos_file_stream_is_valid_(file_stream),
               "Expected a valid file stream stream");

    const size_t fread_result = fread(buffer,
                                      1,
                                      size,
                                      file_stream->file);
    if (fread_result < size) {
        const bool is_error = ferror(file_stream->file) != 0;
        if (is_error) {
            COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_IO,
                                               "Error reading from file"),
                                out_error);
        }
    }

    return fread_result;
}

static size_t
cos_file_stream_write_(CosStream *stream,
                       const void *buffer,
                       size_t size,
                       CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(stream != NULL);
    COS_IMPL_PARAM_CHECK(buffer != NULL);
    COS_IMPL_PARAM_CHECK(size > 0);

    CosFileStream * const file_stream = (CosFileStream *)stream;
    COS_ASSERT(cos_file_stream_is_valid_(file_stream),
               "Expected a valid file stream stream");

    const size_t fwrite_result = fwrite(buffer,
                                        1,
                                        size,
                                        file_stream->file);
    if (fwrite_result < size) {
        const bool is_error = ferror(file_stream->file) != 0;
        if (is_error) {
            COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_IO,
                                               "Error writing to file"),
                                out_error);
        }
    }

    return fwrite_result;
}

static bool
cos_file_stream_seek_(CosStream *stream,
                      CosStreamOffset offset,
                      CosStreamOffsetWhence whence,
                      CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(stream != NULL);

    CosFileStream * const file_stream = (CosFileStream *)stream;
    COS_ASSERT(cos_file_stream_is_valid_(file_stream),
               "Expected a valid file stream stream");

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

    const int fseek_result = cos_fseek(file_stream->file,
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
cos_file_stream_tell_(CosStream *stream,
                      CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(stream != NULL);

    CosFileStream * const file_stream = (CosFileStream *)stream;
    COS_ASSERT(cos_file_stream_is_valid_(file_stream),
               "Expected a valid file stream stream");

    const long offset = ftell(file_stream->file);
    if (offset < 0) {
        //const int ftell_errno = errno;
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_IO,
                                           "Error getting file offset"),
                            out_error);
        return -1;
    }

    return offset;
}

static bool
cos_file_stream_eof_(CosStream *stream)
{
    COS_IMPL_PARAM_CHECK(stream != NULL);

    CosFileStream * const file_stream = (CosFileStream *)stream;
    COS_ASSERT(cos_file_stream_is_valid_(file_stream),
               "Expected a valid file stream stream");

    const bool is_eof = feof(file_stream->file) != 0;
    return is_eof;
}

static void
cos_file_stream_close_(CosStream *stream)
{
    COS_IMPL_PARAM_CHECK(stream != NULL);

    CosFileStream * const file_stream = (CosFileStream *)stream;
    COS_ASSERT(cos_file_stream_is_valid_(file_stream),
               "Expected a valid file stream stream");

    // Close the file only if it is owned by the stream.
    if (file_stream->file_owner) {
        (void)fclose(file_stream->file);
    }
}

COS_ASSUME_NONNULL_END
