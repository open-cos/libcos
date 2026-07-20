/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/xref/table/CosXrefSection.h"
#include "libcos/common/memory/CosMemory.h"

#include "common/Assert.h"

#include "libcos/common/CosArray.h"
#include "libcos/xref/table/CosXrefSubsection.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosXrefSection {
    CosArray *subsections;
};

// Frees the subsection pointed to by an array element when the subsections array is destroyed.
static void
cos_xref_subsection_release_callback_(void *item)
{
    COS_IMPL_PARAM_CHECK(item != NULL);
    if (COS_UNLIKELY(!item)) {
        return;
    }

    CosXrefSubsection * const subsection = *(CosXrefSubsection **)item;
    cos_xref_subsection_destroy(subsection);
}

static const CosArrayCallbacks cos_xref_subsection_array_callbacks_ = {
    .release = cos_xref_subsection_release_callback_,
};

CosXrefSection *
cos_xref_section_create(void)
{
    CosXrefSection *section = NULL;
    CosArray *subsections = NULL;

    section = cos_calloc(1, sizeof(CosXrefSection));
    if (COS_UNLIKELY(!section)) {
        goto failure;
    }

    subsections = cos_array_create(sizeof(CosXrefSubsection *),
                                   &cos_xref_subsection_array_callbacks_,
                                   0);
    if (COS_UNLIKELY(!subsections)) {
        goto failure;
    }

    section->subsections = subsections;

    return section;

failure:
    if (section) {
        cos_free(section);
    }
    if (subsections) {
        cos_array_destroy(subsections);
    }
    return NULL;
}

void
cos_xref_section_destroy(CosXrefSection *section)
{
    COS_API_PARAM_CHECK(section != NULL);
    if (COS_UNLIKELY(!section)) {
        return;
    }

    cos_array_destroy(section->subsections);

    cos_free(section);
}

size_t
cos_xref_section_get_subsection_count(const CosXrefSection *section)
{
    COS_API_PARAM_CHECK(section != NULL);
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
    COS_API_PARAM_CHECK(section != NULL);
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
    COS_API_PARAM_CHECK(section != NULL);
    COS_API_PARAM_CHECK(subsection != NULL);
    if (!section || !subsection) {
        return false;
    }

    return cos_array_append_item(section->subsections, &subsection, out_error);
}

COS_ASSUME_NONNULL_END
