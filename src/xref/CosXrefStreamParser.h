/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_XREF_COS_XREF_STREAM_PARSER_H
#define LIBCOS_XREF_COS_XREF_STREAM_PARSER_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * Parses a cross-reference stream object into a cross-reference section.
 *
 * Reads the @c /W field widths and optional @c /Index subsections from the stream dictionary,
 * decodes the stream data (typically FlateDecode + a PNG predictor), and slices the big-endian
 * field table into free (type 0), in-use (type 1), and compressed (type 2) entries.
 *
 * @param xref_stream The xref stream object; its dictionary must have @c /Type /XRef and @c /W .
 * @param out_error On failure, set to describe the error.
 *
 * @return A new section (destroy with @c cos_xref_section_destroy), or @c NULL on error.
 */
CosXrefSection * COS_Nullable
cos_xref_stream_parse_section_(CosStreamObjNode *xref_stream,
                               CosError * COS_Nullable out_error);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_XREF_COS_XREF_STREAM_PARSER_H */
