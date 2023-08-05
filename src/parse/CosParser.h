//
// Created by david on 29/06/23.
//

#ifndef LIBCOS_COS_PARSER_H
#define LIBCOS_COS_PARSER_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/io/CosInputStream.h>

#include <stdbool.h>

struct CosParser;
typedef struct CosParser CosParser;

CosParser *
cos_parser_alloc(CosInputStream *input_stream);

void
cos_parser_free(CosParser *parser);

bool
cos_parser_parse(CosParser *parser,
                 CosError **error);

#endif /* LIBCOS_COS_PARSER_H */
