/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/xref/table/CosXrefSubsection.h"

#include "common/Assert.h"

#include "libcos/common/CosArray.h"
#include "libcos/common/CosError.h"
#include "libcos/xref/table/CosXrefEntry.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosXrefSubsection {
    CosObjNumber first_object_number;
    size_t entry_count;

    CosArray *entries;
};

CosXrefSubsection *
cos_xref_subsection_create(CosObjNumber first_object_number,
                           size_t entry_count,
                           CosArray * COS_Nullable existing_entries)
{
    CosXrefSubsection *subsection = NULL;
    CosArray *entries = NULL;

    subsection = calloc(1, sizeof(CosXrefSubsection));
    if (COS_UNLIKELY(!subsection)) {
        goto failure;
    }

    if (existing_entries) {
        entries = existing_entries;
    }
    else {
        entries = cos_array_create(sizeof(CosXrefEntry *),
                                   NULL,
                                   entry_count);
        if (COS_UNLIKELY(!entries)) {
            goto failure;
        }
    }

    subsection->first_object_number = first_object_number;
    subsection->entry_count = entry_count;
    subsection->entries = entries;

    return subsection;

failure:
    if (subsection) {
        free(subsection);
    }
    if (entries) {
        cos_array_destroy(entries);
    }
    return NULL;
}

void
cos_xref_subsection_destroy(CosXrefSubsection *subsection)
{
    COS_PARAMETER_ASSERT(subsection != NULL);
    if (COS_UNLIKELY(!subsection)) {
        return;
    }

    cos_array_destroy(subsection->entries);
    free(subsection);
}

CosObjNumber
cos_xref_subsection_get_first_object_number(const CosXrefSubsection *subsection)
{
    COS_PARAMETER_ASSERT(subsection != NULL);
    if (COS_UNLIKELY(!subsection)) {
        return 0;
    }

    return subsection->first_object_number;
}

size_t
cos_xref_subsection_get_entry_count(const CosXrefSubsection *subsection)
{
    COS_PARAMETER_ASSERT(subsection != NULL);
    if (COS_UNLIKELY(!subsection)) {
        return 0;
    }

    return subsection->entry_count;
}

const CosXrefEntry *
cos_xref_subsection_get_entry(const CosXrefSubsection *subsection,
                              size_t index,
                              CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(subsection != NULL);
    if (COS_UNLIKELY(!subsection)) {
        return NULL;
    }

    if (COS_UNLIKELY(index >= subsection->entry_count)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_OUT_OF_RANGE,
                                           "Index out of range"),
                            out_error);
        goto failure;
    }

    CosXrefEntry *entry = NULL;

    if (COS_UNLIKELY(!cos_array_get_item(subsection->entries,
                                         index,
                                         (void *)&entry,
                                         out_error))) {
        goto failure;
    }

    return entry;

failure:

    return NULL;
}

COS_ASSUME_NONNULL_END
