/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/xref/table/CosXrefTable.h"
#include "libcos/common/memory/CosMemory.h"

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

// Frees the section pointed to by an array element when the sections array is destroyed.
static void
cos_xref_section_release_callback_(void *item)
{
    COS_IMPL_PARAM_CHECK(item != NULL);
    if (COS_UNLIKELY(!item)) {
        return;
    }

    CosXrefSection * const section = *(CosXrefSection **)item;
    cos_xref_section_destroy(section);
}

static const CosArrayCallbacks cos_xref_section_array_callbacks_ = {
    .release = cos_xref_section_release_callback_,
};

CosXrefTable *
cos_xref_table_create(void)
{
    CosXrefTable *table = NULL;
    CosArray *sections = NULL;

    table = cos_calloc(1, sizeof(CosXrefTable));
    if (COS_UNLIKELY(!table)) {
        goto failure;
    }

    sections = cos_array_create(sizeof(CosXrefSection *),
                                &cos_xref_section_array_callbacks_,
                                0);
    if (COS_UNLIKELY(!sections)) {
        goto failure;
    }

    table->sections = sections;

    return table;

failure:
    if (table) {
        cos_free(table);
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
    cos_free(table);
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

bool
cos_xref_table_find_next_offset_above(const CosXrefTable *table,
                                      CosStreamOffset offset,
                                      CosStreamOffset *out_next_offset)
{
    COS_API_PARAM_CHECK(table != NULL);
    COS_API_PARAM_CHECK(out_next_offset != NULL);
    if (COS_UNLIKELY(!table || !out_next_offset)) {
        return false;
    }

    bool found = false;
    CosStreamOffset best = 0;

    const size_t section_count = cos_xref_table_get_section_count(table);
    for (size_t si = 0; si < section_count; si++) {
        CosXrefSection * const section = cos_xref_table_get_section(table, si, NULL);
        if (!section) {
            continue;
        }

        const size_t subsection_count = cos_xref_section_get_subsection_count(section);
        for (size_t ssi = 0; ssi < subsection_count; ssi++) {
            CosXrefSubsection * const sub = cos_xref_section_get_subsection(section, ssi, NULL);
            if (!sub) {
                continue;
            }

            const size_t count = cos_xref_subsection_get_entry_count(sub);
            for (size_t ei = 0; ei < count; ei++) {
                const CosXrefEntry * const entry = cos_xref_subsection_get_entry(sub, ei, NULL);
                if (!entry || entry->type != CosXrefEntryType_InUse) {
                    continue;
                }

                const CosStreamOffset entry_offset =
                    (CosStreamOffset)entry->value.in_use.byte_offset;
                if (entry_offset > offset && (!found || entry_offset < best)) {
                    best = entry_offset;
                    found = true;
                }
            }
        }
    }

    if (found) {
        *out_next_offset = best;
    }

    return found;
}

COS_ASSUME_NONNULL_END
