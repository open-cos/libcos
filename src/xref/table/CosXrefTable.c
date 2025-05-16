/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/xref/table/CosXrefTable.h"

#include "common/Assert.h"

#include "libcos/common/CosArray.h"
#include "libcos/xref/table/CosXrefEntry.h"
#include "libcos/xref/table/CosXrefSection.h"
#include "libcos/xref/table/CosXrefSubsection.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosXrefTable {
    CosArray *sections;
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

size_t
cos_xref_table_get_section_count(const CosXrefTable *table)
{
    COS_PARAMETER_ASSERT(table != NULL);
    if (COS_UNLIKELY(!table)) {
        return 0;
    }

    return cos_array_get_count(table->sections);
}

CosXrefSection *
cos_xref_table_get_section(const CosXrefTable *table,
                           size_t index,
                           CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(table != NULL);
    if (COS_UNLIKELY(!table)) {
        return NULL;
    }

    CosXrefSection *section = NULL;
    if (!cos_array_get_item(table->sections,
                            index,
                            (void *)&section,
                            out_error)) {
        return NULL;
    }

    return section;
}

CosXrefEntry *
cos_xref_table_find_entry_for_obj_num(const CosXrefTable *table,
                                      CosObjNumber object_number,
                                      CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(table != NULL);
    if (COS_UNLIKELY(!table)) {
        return NULL;
    }

    (void)object_number;
    (void)out_error;

    return NULL;
}

COS_ASSUME_NONNULL_END
