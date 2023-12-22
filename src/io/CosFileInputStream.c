//
// Created by david on 29/05/23.
//

#include "libcos/io/CosFileInputStream.h"

#include <stdio.h>
#include <stdlib.h>

static size_t
cos_file_input_stream_read(CosInputStream *input_stream,
                           void *buffer,
                           size_t count,
                           void *user_data)
{
    CosFileInputStream * const file_input_stream = (CosFileInputStream *)input_stream;
    FILE * const file = file_input_stream->file;

    const size_t read_count = fread(buffer,
                                    1,
                                    count,
                                    file);

    return read_count;
}

static bool
cos_file_input_stream_get_error(CosInputStream *input_stream,
                                void *user_data)
{
    CosFileInputStream * const file_input_stream = (CosFileInputStream *)input_stream;
    FILE * const file = file_input_stream->file;

    return ferror(file) != 0;
}

static bool
cos_file_input_stream_is_eof(CosInputStream *input_stream,
                             void *user_data)
{
    CosFileInputStream * const file_input_stream = (CosFileInputStream *)input_stream;
    FILE * const file = file_input_stream->file;

    return feof(file) != 0;
}

static void
cos_file_input_stream_close(CosInputStream *input_stream,
                            void *user_data)
{
    CosFileInputStream * const file_input_stream = (CosFileInputStream *)input_stream;
    FILE * const file = file_input_stream->file;

    fclose(file);
}

CosFileInputStream *
cos_file_input_stream_open(const char *filename,
                           const char *mode,
                           void *user_data)
{
    FILE *file = NULL;
    CosFileInputStream *input_stream = NULL;

    file = fopen(filename, mode);
    if (!file) {
        // TODO: POSIX - check errno
        goto failure;
    }

    input_stream = malloc(sizeof(CosFileInputStream));
    if (!input_stream) {
        goto failure;
    }

    cos_file_input_stream_init(input_stream,
                               file,
                               user_data);

    return input_stream;

failure:
    if (file) {
        fclose(file);
    }
    if (input_stream) {
        free(input_stream);
    }
    return NULL;
}

void
cos_file_input_stream_init(CosFileInputStream *self,
                           FILE *file,
                           void *user_data)
{
    cos_input_stream_init((CosInputStream *)self,
                          (CosInputStreamFunctions){
                              .read_func = cos_file_input_stream_read,
                              .error_func = cos_file_input_stream_get_error,
                              .eof_func = cos_file_input_stream_is_eof,
                              .close_func = cos_file_input_stream_close,
                          },
                          user_data);

    self->file = file;
}
