//
// Created by david on 30/05/23.
//

#include "libcos/io/CosInputStream.h"

#include <stdlib.h>
#include <string.h>

typedef struct CosMemoryInputStreamUserData {
    unsigned char *data;
    size_t length;

    size_t offset;
} CosMemoryInputStreamUserData;

static size_t
cos_memory_input_stream_read(CosMemoryInputStream *input_stream,
                             void *buffer,
                             size_t count,
                             void *user_data)
{
    CosMemoryInputStreamUserData * const memory = (CosMemoryInputStreamUserData *)user_data;

    const size_t available_count = memory->length - memory->offset;

    size_t read_count = 0;
    if (count > available_count) {
        read_count = available_count;
    }
    else {
        read_count = count;
    }

    memcpy(buffer,
           memory->data + memory->offset,
           read_count);

    memory->offset += read_count;

    return read_count;
}

static bool
cos_memory_input_stream_error(CosMemoryInputStream *input_stream,
                              void *user_data)
{
    return false;
}

static bool
cos_memory_input_stream_eof(CosMemoryInputStream *input_stream,
                            void *user_data)
{
    const CosMemoryInputStreamUserData * const memory = (CosMemoryInputStreamUserData *)user_data;

    return (memory->offset >= memory->length);
}

static void
cos_memory_input_stream_close(CosMemoryInputStream *input_stream,
                              void *user_data)
{
    CosMemoryInputStreamUserData * const memory = (CosMemoryInputStreamUserData *)user_data;

    //free(memory->data);

    free(memory);
}

CosMemoryInputStream *
cos_memory_input_stream_open(unsigned char *data,
                             size_t length)
{
    const CosInputStreamFunctions functions = {
        .read_func = cos_memory_input_stream_read,
        .error_func = cos_memory_input_stream_error,
        .eof_func = cos_memory_input_stream_eof,
        .close_func = cos_memory_input_stream_close,
    };

    CosMemoryInputStreamUserData * const user_data = malloc(sizeof(CosMemoryInputStreamUserData));
    if (!user_data) {
        return NULL;
    }

    user_data->data = data;
    user_data->length = length;
    user_data->offset = 0;

    CosInputStream * const input_stream = cos_input_stream_open(functions,
                                                                user_data);
    if (!input_stream) {
        free(user_data);
        return NULL;
    }

    return input_stream;
}
