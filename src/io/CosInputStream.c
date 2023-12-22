//
// Created by david on 28/05/23.
//

#include "libcos/io/CosInputStream.h"

#include "common/Assert.h"

#include <stdlib.h>

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

    cos_input_stream_init(input_stream,
                          functions,
                          user_data);

    return input_stream;
}

void
cos_input_stream_init(CosInputStream *self,
                      CosInputStreamFunctions functions,
                      void *user_data)
{
    COS_PARAMETER_ASSERT(self != NULL);
    if (!self) {
        return;
    }

    self->functions = functions;
    self->user_data = user_data;
}

size_t
cos_input_stream_read(CosInputStream *input_stream,
                      void *buffer,
                      size_t count)
{
    COS_PARAMETER_ASSERT(input_stream != NULL);
    COS_PARAMETER_ASSERT(buffer != NULL);
    COS_PARAMETER_ASSERT(count > 0);
    if (!input_stream || !buffer) {
        return 0;
    }

    return input_stream->functions.read_func(input_stream,
                                             buffer,
                                             count,
                                             input_stream->user_data);
}

int
cos_input_stream_get_error(CosInputStream *input_stream)
{
    COS_PARAMETER_ASSERT(input_stream != NULL);
    if (!input_stream) {
        return 1;
    }

    return input_stream->functions.error_func(input_stream,
                                              input_stream->user_data);
}

bool
cos_input_stream_is_eof(CosInputStream *input_stream)
{
    COS_PARAMETER_ASSERT(input_stream != NULL);
    if (!input_stream) {
        return true;
    }

    return input_stream->functions.eof_func(input_stream,
                                            input_stream->user_data);
}

void
cos_input_stream_close(CosInputStream *input_stream)
{
    COS_PARAMETER_ASSERT(input_stream != NULL);
    if (!input_stream) {
        return;
    }

    input_stream->functions.close_func(input_stream,
                                       input_stream->user_data);

    free(input_stream);
}
