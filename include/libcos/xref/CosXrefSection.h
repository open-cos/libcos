/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_XREF_COS_XREF_SECTION_H
#define LIBCOS_XREF_COS_XREF_SECTION_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief Allocates a new cross-reference section.
 *
 * @return A new cross-reference section, or @c NULL if an error occurred.
 */
CosXrefSection * COS_Nullable
cos_xref_section_alloc(void)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

/**
 * @brief Frees a cross-reference section.
 * @param section The cross-reference section to free.
 */
void
cos_xref_section_free(CosXrefSection *section);

/**
 * @brief Gets the number of subsections in a cross-reference section.
 * @param section The cross-reference section.
 *
 * @return The number of subsections in the cross-reference section.
 */
size_t
cos_xref_section_get_subsection_count(const CosXrefSection *section);

/**
 * @brief Gets a subsection from a cross-reference section.
 *
 * @param section The cross-reference section.
 * @param index The index of the subsection to get.
 * @param out_error On input, a pointer to an error object, or @c NULL.
 *
 * @return The subsection at the specified index, or @c NULL if an error occurred.
 */
CosXrefSubsection * COS_Nullable
cos_xref_section_get_subsection(const CosXrefSection *section,
                                size_t index,
                                CosError * COS_Nullable out_error);

/**
 * @brief Adds a subsection to a cross-reference section.
 *
 * @param section The cross-reference section.
 * @param subsection The subsection to add.
 * @param out_error On input, a pointer to an error object, or @c NULL.
 *
 * @return @c true if the subsection was added, or @c false if an error occurred.
 */
bool
cos_xref_section_add_subsection(CosXrefSection *section,
                                CosXrefSubsection *subsection,
                                CosError * COS_Nullable out_error);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_XREF_COS_XREF_SECTION_H */
