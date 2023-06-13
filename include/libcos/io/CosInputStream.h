//
// Created by david on 28/05/23.
//

#ifndef LIBCOS_COS_INPUT_STREAM_H
#define LIBCOS_COS_INPUT_STREAM_H

#include <stdbool.h>
#include <stddef.h>

struct CosInputStream;
typedef struct CosInputStream CosInputStream;

struct CosInputStreamFunctions;
typedef struct CosInputStreamFunctions CosInputStreamFunctions;

struct CosInputStreamFunctions {

    size_t (*read_func)(CosInputStream *input_stream,
                        void *buffer,
                        size_t count,
                        void *user_data);

    bool (*error_func)(CosInputStream *input_stream,
                       void *user_data);

    bool (*eof_func)(CosInputStream *input_stream,
                     void *user_data);

    void (*close_func)(CosInputStream *input_stream,
                       void *user_data);
};

CosInputStream *
cos_input_stream_open(CosInputStreamFunctions functions,
                      void *user_data);

size_t
cos_input_stream_read(CosInputStream *input_stream,
                      void *buffer,
                      size_t count);

int
cos_input_stream_getc(CosInputStream *input_stream);

int
cos_input_stream_ungetc(CosInputStream *input_stream,
                        char character);

int
cos_input_stream_peek(CosInputStream *input_stream);

char *
cos_input_stream_read_line(CosInputStream *input_stream);

int
cos_input_stream_get_error(CosInputStream *input_stream);

bool
cos_input_stream_is_eof(CosInputStream *input_stream);

void
cos_input_stream_close(CosInputStream *input_stream);

typedef CosInputStream CosFileInputStream;

CosFileInputStream *
cos_file_input_stream_open(const char *filename,
                           const char *mode);

typedef CosInputStream CosMemoryInputStream;

CosMemoryInputStream *
cos_memory_input_stream_open(unsigned char *data,
                             size_t length);

#endif /* LIBCOS_COS_INPUT_STREAM_H */
