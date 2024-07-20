/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_XREF_COS_XREF_SUBSECTION_H
#define LIBCOS_XREF_COS_XREF_SUBSECTION_H

#include <libcos/common/CosBasicTypes.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

void
cos_xref_subsection_destroy(CosXrefSubsection *subsection)
    COS_DEALLOCATOR_FUNC;

CosXrefSubsection * COS_Nullable
cos_xref_subsection_create(CosObjNumber first_object_number,
                           size_t entry_count)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_xref_subsection_destroy);

CosObjNumber
cos_xref_subsection_get_first_object_number(const CosXrefSubsection *subsection);

size_t
cos_xref_subsection_get_entry_count(const CosXrefSubsection *subsection);

bool
cos_xref_subsection_has_entry(const CosXrefSubsection *subsection,
                              size_t index);

CosXrefEntry * COS_Nullable
cos_xref_subsection_get_entry(const CosXrefSubsection *subsection,
                              size_t index,
                              CosError * COS_Nullable out_error);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_XREF_COS_XREF_SUBSECTION_H */
