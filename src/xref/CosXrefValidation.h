/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_XREF_COS_XREF_VALIDATION_H
#define LIBCOS_XREF_COS_XREF_VALIDATION_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/syntax/CosLimits.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * Checks that a cross-reference subsection covers a range of object numbers
 * that a document could actually use.
 *
 * A subsection header is a claim made by the file, not a fact about it, so it
 * has to be checked before the count is used to size or bound anything. A
 * subsection covers [@p first_object_number, @p first_object_number + @p
 * entry_count), and ISO 32000-1 Annex C puts the largest object number at
 * @c COS_INDIRECT_OBJ_MAX_NUMBER, so the whole range has to fit below it.
 *
 * Shared by the classic table parser and the xref stream parser, which take
 * their counts from the subsection header and from @c /Index respectively.
 *
 * @param first_object_number The first object number the subsection covers.
 * @param entry_count The number of entries the subsection claims.
 * @param out_error On failure, set to describe the error.
 *
 * @return @c true if the range is usable, @c false otherwise.
 */
COS_STATIC_INLINE bool
cos_xref_validate_subsection_range_(unsigned int first_object_number,
                                    unsigned int entry_count,
                                    CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

COS_STATIC_INLINE bool
cos_xref_validate_subsection_range_(unsigned int first_object_number,
                                    unsigned int entry_count,
                                    CosError * COS_Nullable out_error)
{
    // One more than the largest object number: object 0 heads the free list,
    // so a full table holds COS_INDIRECT_OBJ_MAX_NUMBER + 1 entries.
    const unsigned int max_entry_count = (unsigned int)COS_INDIRECT_OBJ_MAX_NUMBER + 1u;

    // Compared by subtraction so that the range's end cannot overflow.
    if (first_object_number > (unsigned int)COS_INDIRECT_OBJ_MAX_NUMBER ||
        entry_count > max_entry_count ||
        first_object_number > max_entry_count - entry_count) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_XREF,
                                           "Cross-reference subsection covers an "
                                           "impossible range of object numbers"),
                            out_error);
        return false;
    }

    return true;
}

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_XREF_COS_XREF_VALIDATION_H */
