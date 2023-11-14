//
// Created by david on 18/05/23.
//

#include "libcos/CosDocument.h"

#include "libcos/CosObj.h"

#include <stdlib.h>

struct CosDocument {
    int version;
    int x;

    CosObj *root;
};

CosDocument *
cos_document_alloc(void)
{
    CosDocument *const doc = malloc(sizeof(CosDocument));
    return doc;
}

void cos_document_free(CosDocument *doc)
{
    free(doc);
}

int cos_document_get_version(CosDocument *doc)
{
    return doc->version;
}

CosObj *
cos_document_get_root(CosDocument *doc)
{
    return doc->root;
}
