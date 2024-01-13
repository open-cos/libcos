//
// Created by david on 18/05/23.
//

#include "libcos/CosDoc.h"

#include "libcos/CosBaseObj.h"

#include <stdlib.h>

struct CosDoc {
    int version;
    int x;

    CosBaseObj *root;
};

CosDoc *
cos_doc_alloc(void)
{
    CosDoc * const doc = malloc(sizeof(CosDoc));
    return doc;
}

void
cos_doc_free(CosDoc *doc)
{
    free(doc);
}

int
cos_doc_get_version(CosDoc *doc)
{
    return doc->version;
}

CosBaseObj *
cos_doc_get_root(CosDoc *doc)
{
    return doc->root;
}

void *
cos_doc_get_object(CosDoc *doc,
                   CosObjID id,
                   CosError * COS_Nullable error)
{
    return NULL;
}
