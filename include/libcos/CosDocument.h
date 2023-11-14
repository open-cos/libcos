//
// Created by david on 18/05/23.
//

#ifndef LIBCOS_COS_DOCUMENT_H
#define LIBCOS_COS_DOCUMENT_H

#include <libcos/CosObj.h>

struct CosObject;

struct CosDocument;
typedef struct CosDocument CosDocument;

CosDocument *
cos_document_alloc(void);

void cos_document_free(CosDocument *doc);

int cos_document_get_version(CosDocument *doc);

struct CosObj *
cos_document_get_root(CosDocument *doc);

#endif //LIBCOS_COS_DOCUMENT_H
