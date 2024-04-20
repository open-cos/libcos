/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/xref/CosXrefSection.h"

#include "common/Assert.h"

#include <libcos/common/CosArray.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosXrefSection {
    CosArray *subsections;
};

CosXrefSection *
cos_xref_section_alloc(void)
{
    CosXrefSection *section = NULL;
    CosArray *subsections = NULL;

    section = calloc(1, sizeof(CosXrefSection));
    if (!section) {
        goto failure;
    }

    subsections = cos_array_create(sizeof(CosXrefSubsection *),
                                   NULL,
                                   0);
    if (!subsections) {
        goto failure;
    }

    section->subsections = subsections;

    return section;

failure:
    if (section) {
        free(section);
    }
    if (subsections) {
        cos_array_destroy(subsections);
    }
    return NULL;
}

void
cos_xref_section_free(CosXrefSection *section)
{
    if (!section) {
        return;
    }

    cos_array_destroy(section->subsections);

    free(section);
}

size_t
cos_xref_section_get_subsection_count(const CosXrefSection *section)
{
    COS_PARAMETER_ASSERT(section != NULL);
    if (!section) {
        return 0;
    }

    return cos_array_get_count(section->subsections);
}

CosXrefSubsection *
cos_xref_section_get_subsection(const CosXrefSection *section,
                                size_t index,
                                CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(section != NULL);
    if (!section) {
        return NULL;
    }

    CosXrefSubsection *subsection = NULL;
    if (!cos_array_get_item(section->subsections,
                            index,
                            &subsection,
                            out_error)) {
        return NULL;
    }
    return subsection;
}

bool
cos_xref_section_add_subsection(CosXrefSection *section,
                                CosXrefSubsection *subsection,
                                CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(section != NULL);
    COS_PARAMETER_ASSERT(subsection != NULL);
    if (!section || !subsection) {
        return false;
    }

    return cos_array_append_item(section->subsections, subsection, out_error);
}

COS_ASSUME_NONNULL_END
