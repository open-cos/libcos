//
// Created by david on 29/06/23.
//

#ifndef LIBCOS_COS_PARSER_H
#define LIBCOS_COS_PARSER_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosTypes.h>
#include <libcos/io/CosInputStream.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosParser;

CosParser * COS_Nullable
cos_parser_alloc(CosDocument *document,
                 CosInputStream *input_stream)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_parser_free(CosParser *parser);

CosBaseObj * COS_Nullable
cos_parser_next_object(CosParser *parser,
                       CosError * COS_Nullable error)
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_PARSER_H */
