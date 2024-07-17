/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/xref/table/CosXrefTable.h"

#include "common/Assert.h"

#include "libcos/xref/table/CosXrefEntry.h"
#include "libcos/xref/table/CosXrefSection.h"
#include "libcos/xref/table/CosXrefSubsection.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosXrefTable {
    unsigned char padding_[4];
};

CosXrefTable *
cos_xref_table_create(void)
{
    CosXrefTable *table = NULL;

    table = calloc(1, sizeof(CosXrefTable));
    if (COS_UNLIKELY(!table)) {
        return NULL;
    }

    return table;
}

void
cos_xref_table_destroy(CosXrefTable *table)
{
    COS_PARAMETER_ASSERT(table != NULL);
    if (COS_UNLIKELY(!table)) {
        return;
    }

    free(table);
}

COS_ASSUME_NONNULL_END
