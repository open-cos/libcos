/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_PARSE_COS_TOKEN_ENTRY_H
#define LIBCOS_PARSE_COS_TOKEN_ENTRY_H

#include "libcos/common/CosDefines.h"

#include "CosToken.h"
#include "CosTokenValue.h"

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef struct CosTokenEntry {
    CosToken token;
    CosTokenValue *value;
} CosTokenEntry;

CosTokenEntry * COS_Nullable
cos_token_entry_alloc(void)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_token_entry_free(CosTokenEntry *entry);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_PARSE_COS_TOKEN_ENTRY_H */
