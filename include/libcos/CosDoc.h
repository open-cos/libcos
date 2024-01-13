//
// Created by david on 18/05/23.
//

#ifndef LIBCOS_COS_DOC_H
#define LIBCOS_COS_DOC_H

#include <libcos/CosObjID.h>
#include <libcos/common/CosTypes.h>

CosDoc *
cos_doc_alloc(void);

void
cos_doc_free(CosDoc *doc);

int
cos_doc_get_version(CosDoc *doc);

CosBaseObj *
cos_doc_get_root(CosDoc *doc);

void * COS_Nullable
cos_doc_get_object(CosDoc *doc,
                   CosObjID id,
                   CosError * COS_Nullable error);

#endif /* LIBCOS_COS_DOC_H */
