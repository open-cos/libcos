/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_XREF_COS_XREF_SUBSECTION_H
#define LIBCOS_XREF_COS_XREF_SUBSECTION_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

struct CosXrefSubsection {
    unsigned int first_object_number;
    unsigned int entry_count;
    CosXrefEntry *entries;
};

CosXrefSubsection * COS_Nullable
cos_xref_subsection_alloc(unsigned int first_object_number,
                          unsigned int entry_count)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_xref_subsection_free(CosXrefSubsection *subsection);

void
cos_xref_subsection_init(CosXrefSubsection *subsection,
                         unsigned int first_object_number,
                         unsigned int entry_count);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_XREF_COS_XREF_SUBSECTION_H */
