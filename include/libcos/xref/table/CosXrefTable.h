/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_XREF_COS_XREF_TABLE_H
#define LIBCOS_XREF_COS_XREF_TABLE_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

void
cos_xref_table_destroy(CosXrefTable *table)
    COS_DEALLOCATOR_FUNC;

CosXrefTable * COS_Nullable
cos_xref_table_create(void)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_xref_table_destroy);

size_t
cos_xref_table_get_section_count(const CosXrefTable *table);

CosXrefSection * COS_Nullable
cos_xref_table_get_section(const CosXrefTable *table,
                           size_t index,
                           CosError * COS_Nullable out_error);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_XREF_COS_XREF_TABLE_H */
