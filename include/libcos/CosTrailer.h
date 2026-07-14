/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_COS_TRAILER_H
#define LIBCOS_COS_TRAILER_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosBasicTypes.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * A cross-reference trailer for one revision of a document.
 *
 * A file with incremental updates has one trailer per revision. Trailers form a singly linked
 * list ordered newest to oldest; @c cos_trailer_get_prev walks toward older revisions along the
 * @c /Prev chain.
 */

COS_API void
cos_trailer_destroy(CosTrailer *trailer)
    COS_DEALLOCATOR_FUNC;

/**
 * Creates a trailer node.
 *
 * @param dict The trailer dictionary (ownership transferred).
 * @param xref_offset The byte offset of the cross-reference section this trailer belongs to.
 *
 * @return A new trailer, or @c NULL on allocation failure.
 */
COS_API CosTrailer * COS_Nullable
cos_trailer_create(CosDictObjNode *dict,
                   CosStreamOffset xref_offset)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_trailer_destroy)
    COS_OWNERSHIP_TAKES(1);

/**
 * Returns the trailer dictionary.
 */
COS_API CosDictObjNode * COS_Nullable
cos_trailer_get_dict(const CosTrailer *trailer);

/**
 * Returns the byte offset of the cross-reference section this trailer belongs to.
 */
COS_API CosStreamOffset
cos_trailer_get_xref_offset(const CosTrailer *trailer);

/**
 * Returns the previous (older) trailer in the revision chain, or @c NULL if this is the oldest.
 */
COS_API CosTrailer * COS_Nullable
cos_trailer_get_prev(const CosTrailer *trailer);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_TRAILER_H */
