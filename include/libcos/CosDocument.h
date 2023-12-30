//
// Created by david on 18/05/23.
//

#ifndef LIBCOS_COS_DOCUMENT_H
#define LIBCOS_COS_DOCUMENT_H

#include <libcos/CosObjID.h>
#include <libcos/common/CosTypes.h>

CosDocument *
cos_document_alloc(void);

void
cos_document_free(CosDocument *doc);

int
cos_document_get_version(CosDocument *doc);

CosBaseObj *
cos_document_get_root(CosDocument *doc);

CosBaseObj *
cos_document_get_object(CosDocument *doc,
                        CosObjID id,
                        CosError * COS_Nullable error);

#endif //LIBCOS_COS_DOCUMENT_H
