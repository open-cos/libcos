/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "CosTokenEntry.h"

#include "common/Assert.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

CosTokenEntry *
cos_token_entry_alloc(void)
{
    CosTokenEntry *entry = NULL;
    CosTokenValue *value = NULL;

    entry = calloc(1, sizeof(CosTokenEntry));
    if (!entry) {
        goto failure;
    }

    value = cos_token_value_alloc();
    if (!value) {
        goto failure;
    }

    entry->value = value;

    return entry;

failure:
    if (entry) {
        free(entry);
    }
    if (value) {
        cos_token_value_free(value);
    }
    return NULL;
}

void
cos_token_entry_free(CosTokenEntry *entry)
{
    if (!entry) {
        return;
    }

    CosTokenValue *value = entry->value;
    if (value) {
        cos_token_value_free(value);
    }
    free(entry);
}

COS_ASSUME_NONNULL_END
