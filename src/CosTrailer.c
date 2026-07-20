/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/CosTrailer.h"

#include "CosTrailer-Private.h"
#include "common/Assert.h"

#include <libcos/common/memory/CosMemory.h>
#include <libcos/objects/CosDictObjNode.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosTrailer {
    CosDictObjNode *dict;
    CosStreamOffset xref_offset;
    CosTrailer * COS_Nullable prev;
};

CosTrailer *
cos_trailer_create(CosDictObjNode *dict,
                   CosStreamOffset xref_offset)
{
    CosTrailer * const trailer = cos_malloc(sizeof(CosTrailer));
    if (!trailer) {
        return NULL;
    }

    trailer->dict = dict;
    trailer->xref_offset = xref_offset;
    trailer->prev = NULL;

    return trailer;
}

void
cos_trailer_destroy(CosTrailer *trailer)
{
    // Iteratively free the chain to avoid deep recursion on long revision histories.
    CosTrailer * COS_Nullable current = trailer;
    while (current) {
        CosTrailer * const prev = current->prev;
        cos_dict_obj_node_destroy(current->dict);
        cos_free(current);
        current = prev;
    }
}

CosDictObjNode *
cos_trailer_get_dict(const CosTrailer *trailer)
{
    COS_API_PARAM_CHECK(trailer != NULL);
    if (COS_UNLIKELY(!trailer)) {
        return NULL;
    }

    return trailer->dict;
}

CosStreamOffset
cos_trailer_get_xref_offset(const CosTrailer *trailer)
{
    COS_API_PARAM_CHECK(trailer != NULL);
    if (COS_UNLIKELY(!trailer)) {
        return -1;
    }

    return trailer->xref_offset;
}

CosTrailer *
cos_trailer_get_prev(const CosTrailer *trailer)
{
    COS_API_PARAM_CHECK(trailer != NULL);
    if (COS_UNLIKELY(!trailer)) {
        return NULL;
    }

    return trailer->prev;
}

void
cos_trailer_set_prev_(CosTrailer *trailer,
                      CosTrailer * COS_Nullable prev)
{
    COS_IMPL_PARAM_CHECK(trailer != NULL);

    trailer->prev = prev;
}

COS_ASSUME_NONNULL_END
