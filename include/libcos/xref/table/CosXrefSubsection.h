/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_XREF_COS_XREF_SUBSECTION_H
#define LIBCOS_XREF_COS_XREF_SUBSECTION_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

void
cos_xref_subsection_destroy(CosXrefSubsection *subsection)
    COS_DEALLOCATOR_FUNC;

CosXrefSubsection * COS_Nullable
cos_xref_subsection_create(unsigned int first_object_number,
                           unsigned int entry_count)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_xref_subsection_destroy);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_XREF_COS_XREF_SUBSECTION_H */
