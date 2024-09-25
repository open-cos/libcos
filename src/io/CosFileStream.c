/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/io/CosFileStream.h"

#include "common/Assert.h"

#include <libcos/common/CosError.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static bool
cos_file_stream_is_valid_(const CosFileStreamContext *context);

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

static bool
cos_file_stream_eof_(void *context);

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

    CosFileStreamContext *context = NULL;
    FILE *file = NULL;

    context = calloc(1, sizeof(CosFileStreamContext));
    if (COS_UNLIKELY(!context)) {
        goto failure;
    }

    file = fopen(path, mode);
    if (COS_UNLIKELY(!file)) {
        goto failure;
    }

    context->file = file;
    context->file_owner = true;

    const CosStreamFunctions functions = {
        .read_func = &cos_file_stream_read_,
        .write_func = &cos_file_stream_write_,
        .seek_func = &cos_file_stream_seek_,
        .tell_func = &cos_file_stream_tell_,
        .eof_func = &cos_file_stream_eof_,
        .close_func = &cos_file_stream_close_,
    };

    CosStream * const stream = cos_stream_create(&functions,
                                                 context);
    if (COS_UNLIKELY(!stream)) {
        goto failure;
    }

    return stream;

failure:
    if (context) {
        free(context);
    }
    if (file) {
        (void)fclose(file);
    }
    return NULL;
}

static bool
cos_file_stream_is_valid_(const CosFileStreamContext *context)
{
    COS_PARAMETER_ASSERT(context != NULL);

    return (context->file != NULL);
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

    CosFileStreamContext * const file_context = context;
    COS_ASSERT(cos_file_stream_is_valid_(file_context),
               "Expected a valid file stream context");

    const size_t fread_result = fread(buffer,
                                      1,
                                      size,
                                      file_context->file);
    if (fread_result < size) {
        const bool is_error = ferror(file_context->file) != 0;
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

    CosFileStreamContext * const file_context = context;
    COS_ASSERT(cos_file_stream_is_valid_(file_context),
               "Expected a valid file stream context");

    const size_t fwrite_result = fwrite(buffer,
                                        1,
                                        size,
                                        file_context->file);
    if (fwrite_result < size) {
        const bool is_error = ferror(file_context->file) != 0;
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

    CosFileStreamContext * const file_context = context;
    COS_ASSERT(cos_file_stream_is_valid_(file_context),
               "Expected a valid file stream context");

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

    const int fseek_result = fseek(file_context->file,
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

    CosFileStreamContext * const file_context = context;
    COS_ASSERT(cos_file_stream_is_valid_(file_context),
               "Expected a valid file stream context");

    const long offset = ftell(file_context->file);
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
cos_file_stream_eof_(void *context)
{
    COS_PARAMETER_ASSERT(context != NULL);

    CosFileStreamContext * const file_context = context;
    COS_ASSERT(cos_file_stream_is_valid_(file_context),
               "Expected a valid file stream context");

    const bool is_eof = feof(file_context->file) != 0;
    return is_eof;
}

static void
cos_file_stream_close_(void *context)
{
    COS_PARAMETER_ASSERT(context != NULL);

    CosFileStreamContext * const file_context = context;
    COS_ASSERT(cos_file_stream_is_valid_(file_context),
               "Expected a valid file stream context");

    // Close the file only if it is owned by the stream.
    if (file_context->file_owner) {
        (void)fclose(file_context->file);
    }

    free(file_context);
}

COS_ASSUME_NONNULL_END
