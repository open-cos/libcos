//
// Created by david on 28/05/23.
//

#include "CosInputStream.h"

#include <stdio.h>
#include <stdlib.h>

struct CosInputStream {
    CosInputStreamFunctions functions;
    void *user_data;
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
cos_input_stream_get(CosInputStream *input_stream)
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

    free(input_stream);
}

