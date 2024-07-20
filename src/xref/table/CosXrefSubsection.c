/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/xref/table/CosXrefSubsection.h"

#include "common/Assert.h"

#include "libcos/xref/table/CosXrefEntry.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosXrefSubsection {
    unsigned int first_object_number;
    unsigned int entry_count;
    CosXrefEntry *entries;
};

CosXrefSubsection *
cos_xref_subsection_create(unsigned int first_object_number,
                           unsigned int entry_count)
{
    CosXrefSubsection *subsection = NULL;

    subsection = calloc(1, sizeof(CosXrefSubsection));
    if (COS_UNLIKELY(!subsection)) {
        goto failure;
    }

    subsection->first_object_number = first_object_number;
    subsection->entry_count = entry_count;
    subsection->entries = calloc(entry_count, sizeof(CosXrefEntry));
    if (COS_UNLIKELY(!subsection->entries)) {
        goto failure;
    }

    return subsection;

failure:
    if (subsection) {
        free(subsection);
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

    free(subsection->entries);
    free(subsection);
}

COS_ASSUME_NONNULL_END
