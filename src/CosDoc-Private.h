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

void
cos_doc_set_trailer_dict_(CosDoc *doc,
                          CosDictObj * COS_Nullable dict)
    COS_OWNERSHIP_HOLDS(2);

void
cos_doc_set_root_(CosDoc *doc,
                  CosObj * COS_Nullable root);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_DOC_PRIVATE_H */
