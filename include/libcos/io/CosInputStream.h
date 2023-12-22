//
// Created by david on 28/05/23.
//

#ifndef LIBCOS_COS_INPUT_STREAM_H
#define LIBCOS_COS_INPUT_STREAM_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

struct CosInputStreamFunctions {
    size_t (*read_func)(CosInputStream *input_stream,
                        void *buffer,
                        size_t count,
                        void * COS_Nullable user_data);

    bool (*error_func)(CosInputStream *input_stream,
                       void * COS_Nullable user_data);

    bool (*eof_func)(CosInputStream *input_stream,
                     void * COS_Nullable user_data);

    void (*close_func)(CosInputStream *input_stream,
                       void * COS_Nullable user_data);
};

struct CosInputStream {
    CosInputStreamFunctions functions;
    void *user_data;
};

CosInputStream *
cos_input_stream_open(CosInputStreamFunctions functions,
                      void *user_data);

void
cos_input_stream_init(CosInputStream *self,
                      CosInputStreamFunctions functions,
                      void * COS_Nullable user_data);

/**
 * @brief Reads up to @p count bytes from the input stream into the buffer.
 *
 * @param input_stream The input stream.
 * @param buffer The output buffer to read into.
 * @param count The number of bytes to read.
 *
 * @return The number of bytes actually read.
 */
size_t
cos_input_stream_read(CosInputStream *input_stream,
                      void *buffer,
                      size_t count)
    COS_ATTR_ACCESS_WRITE_ONLY(2);

/**
 * @brief Returns the error code of the input stream.
 *
 * @param input_stream The input stream.
 *
 * @return The error code of the input stream.
 */
int
cos_input_stream_get_error(CosInputStream *input_stream);

/**
 * @brief Returns whether the input stream is at the end of the file.
 *
 * @param input_stream The input stream.
 *
 * @return @c true if the input stream is at the end of the file, @c false otherwise.
 */
bool
cos_input_stream_is_eof(CosInputStream *input_stream);

/**
 * @brief Closes the input stream.
 *
 * @param input_stream The input stream.
 */
void
cos_input_stream_close(CosInputStream *input_stream);

typedef CosInputStream CosMemoryInputStream;

CosMemoryInputStream *
cos_memory_input_stream_open(unsigned char *data,
                             size_t length);

#endif /* LIBCOS_COS_INPUT_STREAM_H */
