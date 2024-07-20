/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/xref/table/CosXrefSubsection.h"

#include "common/Assert.h"
#include "common/CosArray.h"

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
                           size_t entry_count)
{
    CosXrefSubsection *subsection = NULL;
    CosArray *entries = NULL;

    subsection = calloc(1, sizeof(CosXrefSubsection));
    if (COS_UNLIKELY(!subsection)) {
        goto failure;
    }

    entries = cos_array_create(sizeof(CosXrefEntry *),
                               NULL,
                               entry_count);
    if (COS_UNLIKELY(!entries)) {
        goto failure;
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

COS_ASSUME_NONNULL_END
