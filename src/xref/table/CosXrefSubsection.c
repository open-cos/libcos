/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/xref/table/CosXrefSubsection.h"

#include "common/Assert.h"

#include "libcos/xref/table/CosXrefEntry.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

CosXrefSubsection *
cos_xref_subsection_create(unsigned int first_object_number,
                          unsigned int entry_count)
{
    CosXrefSubsection *subsection = NULL;

    subsection = calloc(1, sizeof(CosXrefSubsection));
    if (!subsection) {
        return NULL;
    }

    subsection->first_object_number = first_object_number;
    subsection->entry_count = entry_count;
    subsection->entries = calloc(entry_count, sizeof(CosXrefEntry));
    if (!subsection->entries) {
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
    if (!subsection) {
        return;
    }

    free(subsection->entries);
    free(subsection);
}

void
cos_xref_subsection_init(CosXrefSubsection *subsection,
                         unsigned int first_object_number,
                         unsigned int entry_count)
{
    COS_PARAMETER_ASSERT(subsection != NULL);
    if (!subsection) {
        return;
    }

    subsection->first_object_number = first_object_number;
    subsection->entry_count = entry_count;
}

COS_ASSUME_NONNULL_END
