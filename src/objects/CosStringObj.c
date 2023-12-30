//
// Created by david on 16/10/23.
//

#include "libcos/CosStringObj.h"

#include "CosStringObj-Impl.h"
#include "common/Assert.h"

#include <libcos/CosBaseObj.h>

#include <stdlib.h>
#include <string.h>

CosStringObj * COS_Nullable
cos_string_obj_alloc(CosData *string_data,
                     CosDocument * COS_Nullable document)
{
    CosStringObj * const obj = cos_obj_alloc(sizeof(CosStringObj),
                                             CosObjectType_String,
                                             document);

    if (!obj) {
        return NULL;
    }

    obj->data = *string_data;

    // Free just the container, not the data itself.
    free(string_data);

    return obj;
}
