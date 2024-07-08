/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/io/CosMemoryStream.h"

#include "common/Assert.h"

#include <libcos/common/CosError.h>

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

typedef struct CosMemoryStreamContext {
    union {
        const unsigned char *read;
        unsigned char *write;
    } buffer;

    size_t size;
    size_t position;
    bool free_buffer;

} CosMemoryStreamContext;

static size_t
cos_memory_stream_read_(void *context,
                        void *buffer,
                        size_t count,
                        CosError * COS_Nullable out_error);

static size_t
cos_memory_stream_write_(void *context,
                         const void *buffer,
                         size_t count,
                         CosError * COS_Nullable out_error);

static bool
cos_memory_stream_seek_(void *context,
                        CosStreamOffset offset,
                        CosStreamOffsetWhence whence,
                        CosError * COS_Nullable out_error);

static CosStreamOffset
cos_memory_stream_tell_(void *context,
                        CosError * COS_Nullable out_error);

static bool
cos_memory_stream_eof_(void *context);

static void
cos_memory_stream_close_(void *context);

CosStream *
cos_memory_stream_create(void *buffer,
                         size_t size,
                         bool free_buffer)
{
    COS_PARAMETER_ASSERT(buffer != NULL);
    if (COS_UNLIKELY(!buffer)) {
        return NULL;
    }

    CosStreamFunctions stream_functions = {
        .read_func = &cos_memory_stream_read_,
        .write_func = &cos_memory_stream_write_,
        .seek_func = &cos_memory_stream_seek_,
        .tell_func = &cos_memory_stream_tell_,
        .eof_func = &cos_memory_stream_eof_,
        .close_func = &cos_memory_stream_close_,
    };

    CosMemoryStreamContext *context = calloc(1, sizeof(CosMemoryStreamContext));
    if (COS_UNLIKELY(!context)) {
        return NULL;
    }

    context->buffer.write = buffer;
    context->size = size;
    context->position = 0;
    context->free_buffer = free_buffer;

    CosStream *stream = cos_stream_create(&stream_functions,
                                          context);
    if (COS_UNLIKELY(!stream)) {
        goto failure;
    }

    return stream;

failure:
    if (context) {
        free(context);
    }
    return NULL;
}

CosStream *
cos_memory_stream_create_readonly(const void *buffer,
                                  size_t size,
                                  bool free_buffer)
{
    COS_PARAMETER_ASSERT(buffer != NULL);
    if (COS_UNLIKELY(!buffer)) {
        return NULL;
    }

    CosStreamFunctions functions = {
        .read_func = &cos_memory_stream_read_,
        .write_func = NULL,
        .seek_func = &cos_memory_stream_seek_,
        .tell_func = &cos_memory_stream_tell_,
        .eof_func = &cos_memory_stream_eof_,
        .close_func = &cos_memory_stream_close_,
    };

    CosMemoryStreamContext *context = calloc(1, sizeof(CosMemoryStreamContext));
    if (COS_UNLIKELY(!context)) {
        return NULL;
    }

    context->buffer.read = buffer;
    context->size = size;
    context->position = 0;
    context->free_buffer = free_buffer;

    return cos_stream_create(&functions, context);
}

static size_t
cos_memory_stream_read_(void *context,
                        void *buffer,
                        size_t count,
                        COS_ATTR_UNUSED CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(context != NULL);
    COS_PARAMETER_ASSERT(buffer != NULL);
    if (count == 0) {
        return 0;
    }

    CosMemoryStreamContext * const memory_stream_context = (CosMemoryStreamContext *)context;

    const size_t size = memory_stream_context->size;
    const size_t position = memory_stream_context->position;

    if (position >= size) {
        return 0;
    }

    const size_t remaining = size - position;
    const size_t read_count = (count < remaining) ? count : remaining;

    const void * const source = memory_stream_context->buffer.read + position;
    void * const destination = buffer;

    memcpy(destination,
           source,
           read_count);

    return read_count;
}

static size_t
cos_memory_stream_write_(void *context,
                         const void *buffer,
                         size_t count,
                         COS_ATTR_UNUSED CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(context != NULL);
    COS_PARAMETER_ASSERT(buffer != NULL);

    if (count == 0) {
        return 0;
    }

    CosMemoryStreamContext * const memory_stream_context = (CosMemoryStreamContext *)context;

    const size_t size = memory_stream_context->size;
    const size_t position = memory_stream_context->position;

    const size_t remaining = size - position;
    const size_t write_count = (count < remaining) ? count : remaining;

    const void * const source = buffer;
    void * const destination = memory_stream_context->buffer.write + position;

    memcpy(destination,
           source,
           write_count);

    return write_count;
}

static bool
cos_memory_stream_seek_(void *context,
                        CosStreamOffset offset,
                        CosStreamOffsetWhence whence,
                        CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(context != NULL);

    CosMemoryStreamContext * const memory_stream_context = (CosMemoryStreamContext *)context;

    const size_t size = memory_stream_context->size;
    const size_t position = memory_stream_context->position;

    switch (whence) {
        case CosStreamOffsetWhence_Set: {
            if (offset < 0) {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                                   "Invalid negative offset"),
                                    out_error);
                return false;
            }
            else if ((size_t)offset > size) {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_OUT_OF_RANGE,
                                                   "Stream offset out of range"),
                                    out_error);
                return false;
            }

            memory_stream_context->position = (size_t)offset;
        } break;
        case CosStreamOffsetWhence_Current: {
            if (offset < 0 && (position < (size_t)-offset)) {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_OUT_OF_RANGE,
                                                   "Invalid negative stream position"),
                                    out_error);
                return false;
            }
            else if (offset > 0 && ((position + (size_t)offset) >= size)) {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_OUT_OF_RANGE,
                                                   "Stream offset out of range"),
                                    out_error);
                return false;
            }

            memory_stream_context->position += (size_t)offset;
        } break;
        case CosStreamOffsetWhence_End: {
            if (offset > 0) {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                                   "Invalid positive offset"),
                                    out_error);
                return false;
            }
            else if ((size_t)-offset > size) {
                COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_OUT_OF_RANGE,
                                                   "Stream offset out of range"),
                                    out_error);
                return false;
            }

            memory_stream_context->position = (size_t)(size + (size_t)offset);
        } break;
    }

    return true;
}

static CosStreamOffset
cos_memory_stream_tell_(void *context,
                        COS_ATTR_UNUSED CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(context != NULL);

    CosMemoryStreamContext * const memory_stream_context = (CosMemoryStreamContext *)context;

    return (CosStreamOffset)memory_stream_context->position;
}

static void
cos_memory_stream_close_(void *context)
{
    COS_PARAMETER_ASSERT(context != NULL);

    CosMemoryStreamContext * const memory_stream_context = (CosMemoryStreamContext *)context;

    if (memory_stream_context->free_buffer) {
        free(memory_stream_context->buffer.write);
    }

    free(memory_stream_context);
}

static bool
cos_memory_stream_eof_(void *context)
{
    COS_PARAMETER_ASSERT(context != NULL);

    CosMemoryStreamContext * const memory_stream_context = (CosMemoryStreamContext *)context;

    return memory_stream_context->position >= memory_stream_context->size;
}

COS_ASSUME_NONNULL_END
