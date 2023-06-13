//
// Created by david on 28/05/23.
//

#include "libcos/io/CosInputStream.h"
#include "common/CosVector.h"

#include <stdio.h>
#include <stdlib.h>

#define COS_INPUT_STREAM_BUFFER_SIZE 1024

struct CosInputStream {
    CosInputStreamFunctions functions;
    void *user_data;

    char *buffer;
    size_t buffer_size;
    size_t buffer_index;
};

CosInputStream *
cos_input_stream_open(CosInputStreamFunctions functions,
                      void *user_data)
{
    if (!functions.read_func || !functions.close_func) {
        return NULL;
    }

    CosInputStream * const input_stream = malloc(sizeof(CosInputStream));
    if (!input_stream) {
        return NULL;
    }

    input_stream->functions = functions;
    input_stream->user_data = user_data;

    input_stream->buffer = malloc(COS_INPUT_STREAM_BUFFER_SIZE * sizeof(char));
    input_stream->buffer_size = COS_INPUT_STREAM_BUFFER_SIZE;
    input_stream->buffer_index = 0;

    return input_stream;
}

size_t
cos_input_stream_read(CosInputStream *input_stream,
                      void *buffer,
                      size_t count)
{
    return input_stream->functions.read_func(input_stream,
                                             buffer,
                                             count,
                                             input_stream->user_data);
}

int
cos_input_stream_getc(CosInputStream *input_stream)
{
    char result;
    const size_t read_count = cos_input_stream_read(input_stream,
                                                    &result,
                                                    1);
    if (read_count < 1) {
        return -1;
    }

    return result;
}

int
cos_input_stream_ungetc(CosInputStream *input_stream,
                        char character)
{

    cos_vector_add_element(input_stream->buffer,
                           &character);

    return character;
}

int
cos_input_stream_peek(CosInputStream *input_stream)
{
    const int result = cos_input_stream_getc(input_stream);
    if (result < 0) {
        return result;
    }

    cos_input_stream_ungetc(input_stream,
                            result);

    return result;
}

char *
cos_input_stream_read_line(CosInputStream *input_stream)
{
    char *result = NULL;

    int character;
    while ((character = cos_input_stream_getc(input_stream)) != EOF) {
        if (character < 0) {
            break;
        }

        switch (character) {
            case '\n': {
                return result;
            }
            case '\r': {
                int peeked_character = 0;
                if ((peeked_character = cos_input_stream_peek(input_stream)) == '\n') {
                    cos_input_stream_getc(input_stream);
                }
                return result;
            }

            default:
                break;
        }
    }

    return result;
}

int
cos_input_stream_get_error(CosInputStream *input_stream)
{
    return input_stream->functions.error_func(input_stream,
                                              input_stream->user_data);
}

bool
cos_input_stream_is_eof(CosInputStream *input_stream)
{
    return input_stream->functions.eof_func(input_stream,
                                            input_stream->user_data);
}

void
cos_input_stream_close(CosInputStream *input_stream)
{
    input_stream->functions.close_func(input_stream,
                                       input_stream->user_data);

    free(input_stream->buffer);

    free(input_stream);
}
