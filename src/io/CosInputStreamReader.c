//
// Created by david on 02/12/23.
//

#include "CosInputStreamReader.h"

#include "common/Assert.h"

#include <stdio.h>
#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

#define COS_INPUT_STREAM_READER_BUFFER_SIZE 1024

struct CosInputStreamReader {
    CosInputStream *input_stream;

    unsigned char *buffer;
    size_t buffer_size;
    size_t buffer_position;
};

static int
cos_input_stream_reader_get_current_(CosInputStreamReader *input_stream_reader);

static void
cos_input_stream_reader_advance_(CosInputStreamReader *input_stream_reader);

static bool
cos_input_stream_reader_backup_(CosInputStreamReader *input_stream_reader);

CosInputStreamReader *
cos_input_stream_reader_alloc(CosInputStream *input_stream)
{
    CosInputStreamReader *reader = NULL;
    unsigned char *buffer = NULL;

    COS_PARAMETER_ASSERT(input_stream != NULL);
    if (!input_stream) {
        goto failure;
    }

    reader = malloc(sizeof(CosInputStreamReader));
    if (!reader) {
        goto failure;
    }

    reader->input_stream = input_stream;

    buffer = malloc(COS_INPUT_STREAM_READER_BUFFER_SIZE * sizeof(unsigned char));
    if (!buffer) {
        goto failure;
    }

    reader->buffer = buffer;
    reader->buffer_size = COS_INPUT_STREAM_READER_BUFFER_SIZE;
    reader->buffer_position = 0;

    return reader;

failure:
    if (reader) {
        free(reader);
    }
    if (buffer) {
        free(buffer);
    }

    return NULL;
}

void
cos_input_stream_reader_free(CosInputStreamReader *input_stream_reader)
{
    if (!input_stream_reader) {
        return;
    }

    free(input_stream_reader->buffer);

    free(input_stream_reader);
}

int
cos_input_stream_reader_getc(CosInputStreamReader *input_stream_reader)
{
    COS_PARAMETER_ASSERT(input_stream_reader != NULL);

    int result = cos_input_stream_reader_get_current_(input_stream_reader);
    cos_input_stream_reader_advance_(input_stream_reader);
    return result;
}

int
cos_input_stream_reader_peek(CosInputStreamReader *input_stream_reader)
{
    COS_PARAMETER_ASSERT(input_stream_reader != NULL);

    return cos_input_stream_reader_get_current_(input_stream_reader);
}

static int
cos_input_stream_reader_get_current_(CosInputStreamReader *input_stream_reader)
{
    COS_PARAMETER_ASSERT(input_stream_reader != NULL);

    if (COS_LIKELY(input_stream_reader->buffer_position < input_stream_reader->buffer_size)) {
        return input_stream_reader->buffer[input_stream_reader->buffer_position];
    }
    else {
        const size_t read_count = cos_input_stream_read(input_stream_reader->input_stream,
                                                        input_stream_reader->buffer,
                                                        input_stream_reader->buffer_size);
        if (read_count == 0) {
            return EOF;
        }

        input_stream_reader->buffer_position = 0;
        return input_stream_reader->buffer[input_stream_reader->buffer_position];
    }
}

static void
cos_input_stream_reader_advance_(CosInputStreamReader *input_stream_reader)
{
    COS_PARAMETER_ASSERT(input_stream_reader != NULL);

    input_stream_reader->buffer_position++;
}

static bool
cos_input_stream_reader_backup_(CosInputStreamReader *input_stream_reader)
{
    COS_PARAMETER_ASSERT(input_stream_reader != NULL);

    if (COS_LIKELY(input_stream_reader->buffer_position > 0)) {
        input_stream_reader->buffer_position--;
        return true;
    }
    else {
        return false;
    }
}

COS_ASSUME_NONNULL_END
