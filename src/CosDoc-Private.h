/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_COS_DOC_PRIVATE_H
#define LIBCOS_COS_DOC_PRIVATE_H

#include <libcos/CosDoc.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

void
cos_doc_set_version_(CosDoc *doc,
                     int version);

void
cos_doc_set_parser_(CosDoc *doc,
                    CosParser * COS_Nullable parser)
    COS_OWNERSHIP_HOLDS(2);

void
cos_doc_set_xref_table_(CosDoc *doc,
                        CosXrefTable * COS_Nullable table)
    COS_OWNERSHIP_HOLDS(2);

/**
 * Returns the document's cross-reference table, or @c NULL if it has not been
 * built yet (as when an xref stream is still being parsed).
 *
 * @param doc The document.
 *
 * @return The cross-reference table, or @c NULL .
 */
const CosXrefTable * COS_Nullable
cos_doc_get_xref_table_(const CosDoc *doc);

void
cos_doc_set_trailer_(CosDoc *doc,
                     CosTrailer * COS_Nullable trailer)
    COS_OWNERSHIP_HOLDS(2);

void
cos_doc_set_root_(CosDoc *doc,
                  CosObjNode * COS_Nullable root);

/**
 * Records the options the document is being parsed with.
 *
 * @param doc The document.
 * @param options The options, or @c NULL for the defaults.
 */
void
cos_doc_set_parser_options_(CosDoc *doc,
                            const CosParserOptions * COS_Nullable options);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_DOC_PRIVATE_H */
