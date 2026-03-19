/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_XREF_COS_XREF_TABLE_H
#define LIBCOS_XREF_COS_XREF_TABLE_H

#include "common/CosBasicTypes.h"

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
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

/**
 * @brief Adds a section to a cross-reference table.
 *
 * @param table The cross-reference table.
 * @param section The section to add.
 * @param out_error On input, a pointer to an error object, or @c NULL.
 *
 * @return @c true if the section was added, or @c false if an error occurred.
 */
bool
cos_xref_table_add_section(CosXrefTable *table,
                           CosXrefSection *section,
                           CosError * COS_Nullable out_error);

const CosXrefEntry * COS_Nullable
cos_xref_table_find_entry_for_obj_num(const CosXrefTable *table,
                                      CosObjNumber object_number,
                                      CosError * COS_Nullable out_error);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_XREF_COS_XREF_TABLE_H */
