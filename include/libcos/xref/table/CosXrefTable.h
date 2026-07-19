/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_XREF_COS_XREF_TABLE_H
#define LIBCOS_XREF_COS_XREF_TABLE_H

#include "common/CosBasicTypes.h"

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

COS_API void
cos_xref_table_destroy(CosXrefTable *table)
    COS_DEALLOCATOR_FUNC;

COS_API CosXrefTable * COS_Nullable
cos_xref_table_create(void)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_xref_table_destroy);

COS_API size_t
cos_xref_table_get_section_count(const CosXrefTable *table);

COS_API CosXrefSection * COS_Nullable
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
COS_API bool
cos_xref_table_add_section(CosXrefTable *table,
                           CosXrefSection *section,
                           CosError * COS_Nullable out_error);

COS_API const CosXrefEntry * COS_Nullable
cos_xref_table_find_entry_for_obj_num(const CosXrefTable *table,
                                      CosObjNumber object_number,
                                      CosError * COS_Nullable out_error);

/**
 * @brief Finds the smallest in-use object byte offset strictly greater than a
 * given offset.
 *
 * Used to bound a stream's extent during /Length recovery: the next object in
 * the file is an upper bound on where the stream's data can end.
 *
 * @param table The cross-reference table.
 * @param offset The offset to search above.
 * @param out_next_offset On success, set to the nearest in-use byte offset
 * greater than @p offset .
 *
 * @return @c true if such an offset was found, or @c false if no in-use entry
 * lies above @p offset .
 */
COS_API bool
cos_xref_table_find_next_offset_above(const CosXrefTable *table,
                                      CosStreamOffset offset,
                                      CosStreamOffset *out_next_offset)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_XREF_COS_XREF_TABLE_H */
