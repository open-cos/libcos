/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "io/CosStreamReader.h"

#include "common/Assert.h"

#include "libcos/io/CosStream.h"

#include <stdio.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

#define COS_STREAM_READER_BUFFER_SIZE 1024

struct CosStreamReader {
    CosStream *input_stream;

    unsigned char *buffer COS_ATTR_NONSTRING;
    size_t buffer_capacity;

    size_t buffer_start;
    size_t buffer_end;
    size_t buffer_position;

    bool is_eof;
    size_t eof_position;
};

static int
cos_stream_reader_get_current_(CosStreamReader *stream_reader);

static void
cos_stream_reader_advance_(CosStreamReader *stream_reader);

static bool
cos_stream_reader_backup_(CosStreamReader *stream_reader);

CosStreamReader *
cos_stream_reader_create(CosStream *input_stream)
{
    CosStreamReader *reader = NULL;
    unsigned char *buffer = NULL;

    COS_PARAMETER_ASSERT(input_stream != NULL);
    if (!input_stream) {
        goto failure;
    }

    reader = malloc(sizeof(CosStreamReader));
    if (!reader) {
        goto failure;
    }

    reader->input_stream = input_stream;

    buffer = malloc(COS_STREAM_READER_BUFFER_SIZE * sizeof(unsigned char));
    if (!buffer) {
        goto failure;
    }

    reader->buffer = buffer;
    reader->buffer_capacity = COS_STREAM_READER_BUFFER_SIZE;

    reader->buffer_start = 0;
    reader->buffer_end = 0;
    reader->buffer_position = reader->buffer_start;

    reader->is_eof = false;
    reader->eof_position = 0;

    return reader;

failure:
    if (reader) {
        free(reader);
    }

    return NULL;
}

void
cos_stream_reader_destroy(CosStreamReader *stream_reader)
{
    COS_PARAMETER_ASSERT(stream_reader != NULL);
    if (!stream_reader) {
        return;
    }

    free(stream_reader->buffer);

    free(stream_reader);
}

void
cos_stream_reader_reset(CosStreamReader *stream_reader)
{
    COS_PARAMETER_ASSERT(stream_reader != NULL);
    if (COS_UNLIKELY(!stream_reader)) {
        return;
    }

    stream_reader->buffer_start = 0;
    stream_reader->buffer_end = 0;
    stream_reader->buffer_position = stream_reader->buffer_start;

    stream_reader->is_eof = false;
    stream_reader->eof_position = 0;
}

bool
cos_stream_reader_is_at_end(CosStreamReader *stream_reader)
{
    COS_PARAMETER_ASSERT(stream_reader != NULL);

    return cos_stream_reader_get_current_(stream_reader) == EOF;
}

int
cos_stream_reader_getc(CosStreamReader *stream_reader)
{
    COS_PARAMETER_ASSERT(stream_reader != NULL);

    int result = cos_stream_reader_get_current_(stream_reader);
    cos_stream_reader_advance_(stream_reader);
    return result;
}

int
cos_stream_reader_peek(CosStreamReader *stream_reader)
{
    COS_PARAMETER_ASSERT(stream_reader != NULL);

    return cos_stream_reader_get_current_(stream_reader);
}

bool
cos_stream_reader_ungetc(CosStreamReader *stream_reader)
{
    COS_PARAMETER_ASSERT(stream_reader != NULL);

    return cos_stream_reader_backup_(stream_reader);
}

static int
cos_stream_reader_get_current_(CosStreamReader *stream_reader)
{
    COS_PARAMETER_ASSERT(stream_reader != NULL);

    if (COS_LIKELY(stream_reader->buffer_position < stream_reader->buffer_end)) {
        return stream_reader->buffer[stream_reader->buffer_position];
    }
    else {
        // Check if the end of the stream was detected previously.
        if (stream_reader->is_eof &&
            stream_reader->buffer_position >= stream_reader->eof_position) {
            return EOF;
        }

        const size_t read_count = stream_reader->buffer_capacity;

        const size_t actual_read_count = cos_stream_read(stream_reader->input_stream,
                                                         stream_reader->buffer,
                                                         read_count,
                                                         NULL);
        stream_reader->buffer_start = 0;
        stream_reader->buffer_end = actual_read_count;
        stream_reader->buffer_position = 0;

        if (actual_read_count < read_count) {
            if (cos_stream_is_at_end(stream_reader->input_stream,
                                     NULL)) {
                stream_reader->is_eof = true;
                stream_reader->eof_position = actual_read_count;
            }
            if (actual_read_count == 0) {
                return EOF;
            }
        }

        return stream_reader->buffer[stream_reader->buffer_position];
    }
}

static void
cos_stream_reader_advance_(CosStreamReader *stream_reader)
{
    COS_PARAMETER_ASSERT(stream_reader != NULL);
    if (!stream_reader) {
        return;
    }

    stream_reader->buffer_position++;
}

static bool
cos_stream_reader_backup_(CosStreamReader *stream_reader)
{
    COS_PARAMETER_ASSERT(stream_reader != NULL);

    if (COS_LIKELY(stream_reader->buffer_position > stream_reader->buffer_start)) {
        stream_reader->buffer_position--;
        return true;
    }
    else {
        return false;
    }
}

COS_ASSUME_NONNULL_END
