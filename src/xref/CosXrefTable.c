/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/xref/CosXrefTable.h"

#include <libcos/xref/CosXrefEntry.h>
#include <libcos/xref/CosXrefSection.h>
#include <libcos/xref/CosXrefSubsection.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosXrefTable {
    unsigned char padding_[4];
};

CosXrefTable *
cos_xref_table_alloc(void)
{
    CosXrefTable *table = NULL;

    table = calloc(1, sizeof(CosXrefTable));
    if (!table) {
        return NULL;
    }

    return table;
}

void
cos_xref_table_free(CosXrefTable *table)
{
    if (!table) {
        return;
    }

    free(table);
}

static void
cos_xref_do_something(void)
{
}

COS_ASSUME_NONNULL_END
