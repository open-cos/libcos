/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_XREF_COS_XREF_TABLE_H
#define LIBCOS_XREF_COS_XREF_TABLE_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

CosXrefTable * COS_Nullable
cos_xref_table_alloc(void)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_xref_table_free(CosXrefTable *table);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_XREF_COS_XREF_TABLE_H */
