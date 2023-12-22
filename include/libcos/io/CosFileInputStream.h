//
// Created by david on 19/12/23.
//

#ifndef LIBCOS_COS_FILE_INPUT_STREAM_H
#define LIBCOS_COS_FILE_INPUT_STREAM_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>
#include <libcos/io/CosInputStream.h>

#include <stdio.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosFileInputStream {
    CosInputStream base;
    FILE *file;
};

CosFileInputStream *
cos_file_input_stream_open(const char *filename,
                           const char *mode,
                           void * COS_Nullable user_data);

void
cos_file_input_stream_init(CosFileInputStream *file_input_stream,
                           FILE *file,
                           void * COS_Nullable user_data);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_FILE_INPUT_STREAM_H */
