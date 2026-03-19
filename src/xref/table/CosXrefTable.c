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

    return cos_array_append_item(table->sections, (void *)&section, out_error);
}

const CosXrefEntry *
cos_xref_table_find_entry_for_obj_num(const CosXrefTable *table,
                                      CosObjNumber object_number,
                                      CosError * COS_Nullable out_error)
{
    COS_API_PARAM_CHECK(table != NULL);
    if (COS_UNLIKELY(!table)) {
        return NULL;
    }

    const size_t section_count = cos_xref_table_get_section_count(table);
    for (size_t si = 0; si < section_count; si++) {
        CosXrefSection * const section = cos_xref_table_get_section(table, si, out_error);
        if (!section) {
            continue;
        }

        const size_t subsection_count = cos_xref_section_get_subsection_count(section);
        for (size_t ssi = 0; ssi < subsection_count; ssi++) {
            CosXrefSubsection * const sub = cos_xref_section_get_subsection(section, ssi, out_error);
            if (!sub) {
                continue;
            }

            const CosObjNumber first = cos_xref_subsection_get_first_object_number(sub);
            const size_t count = cos_xref_subsection_get_entry_count(sub);

            if (object_number >= first && (object_number - first) < (CosObjNumber)count) {
                const size_t index = (size_t)(object_number - first);
                const CosXrefEntry * const entry = cos_xref_subsection_get_entry(sub, index, out_error);
                return entry;
            }
        }
    }

    return NULL;
}

COS_ASSUME_NONNULL_END
