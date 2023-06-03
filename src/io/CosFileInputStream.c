//
// Created by david on 29/05/23.
//

#include "CosInputStream.h"

#include <stdio.h>

static size_t
cos_file_input_stream_read(CosFileInputStream *input_stream,
                           void *buffer,
                           size_t count,
                           void *user_data)
{
    FILE * const file = (FILE *)user_data;

    const size_t read_count = fread(buffer,
                                    1,
                                    count,
                                    file);

    return read_count;
}

static bool
cos_file_input_stream_get_error(CosFileInputStream *input_stream,
                                void *user_data)
{
    FILE * const file = (FILE *)user_data;

    return ferror(file) != 0;
}

static bool
cos_file_input_stream_is_eof(CosFileInputStream *input_stream,
                             void *user_data)
{
    FILE * const file = (FILE *)user_data;

    return feof(file) != 0;
}

static void
cos_file_input_stream_close(CosFileInputStream *input_stream,
                            void *user_data)
{
    FILE * const file = (FILE *)user_data;

    fclose(file);
}

CosFileInputStream *
cos_file_input_stream_open(const char *filename,
                           const char *mode)
{
    FILE * const file = fopen(filename,
                              mode);
    if (!file) {
        // TODO: POSIX - check errno
        return NULL;
    }

    CosInputStreamFunctions functions = {
        .read_func = cos_file_input_stream_read,
        .error_func = cos_file_input_stream_get_error,
        .eof_func = cos_file_input_stream_is_eof,
        .close_func = cos_file_input_stream_close,
    };

    CosInputStream * const input_stream = cos_input_stream_open(functions,
                                                                file);
    if (!input_stream) {
        fclose(file);

        return NULL;
    }

    return input_stream;
}
