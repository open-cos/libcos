/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/xref/table/CosXrefTable.h"

#include "common/Assert.h"

#include "libcos/common/CosArray.h"
#include "libcos/common/CosError.h"
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
    CosArray *sections = NULL;

    table = calloc(1, sizeof(CosXrefTable));
    if (COS_UNLIKELY(!table)) {
        goto failure;
    }

    sections = cos_array_create(sizeof(CosXrefSection *), NULL, 0);
    if (COS_UNLIKELY(!sections)) {
        goto failure;
    }

    table->sections = sections;

    return table;

failure:
    if (table) {
        free(table);
    }
    if (sections) {
        cos_array_destroy(sections);
    }
    return NULL;
}

void
cos_xref_table_destroy(CosXrefTable *table)
{
    COS_API_PARAM_CHECK(table != NULL);
    if (COS_UNLIKELY(!table)) {
        return;
    }

    if (table->sections) {
        cos_array_destroy(table->sections);
    }
    free(table);
}

size_t
cos_xref_table_get_section_count(const CosXrefTable *table)
{
    COS_API_PARAM_CHECK(table != NULL);
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
    COS_API_PARAM_CHECK(table != NULL);
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

bool
cos_xref_table_add_section(CosXrefTable *table,
                           CosXrefSection *section,
                           CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(table != NULL);
    COS_API_PARAM_CHECK(section != NULL);
    if (COS_UNLIKELY(!table || !section)) {
        return false;
    }

    return cos_array_append_item(table->sections, &section, out_error);
}

CosXrefEntry *
cos_xref_table_find_entry_for_obj_num(const CosXrefTable *table,
                                      CosObjNumber object_number,
                                      CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(table != NULL);
    if (COS_UNLIKELY(!table)) {
        return NULL;
    }

    (void)object_number;
    (void)out_error;

    return NULL;
}

COS_ASSUME_NONNULL_END
