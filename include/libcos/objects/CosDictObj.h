/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COS_DICT_OBJ_H
#define LIBCOS_COS_DICT_OBJ_H

#include <libcos/CosBaseObj.h>
#include <libcos/common/CosDefines.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef struct CosDict CosDict;
typedef struct CosDictEntry CosDictEntry;

struct CosDictEntry {
    CosBaseObj *key;
    CosBaseObj *value;
};

struct CosDict {
    CosDictEntry * COS_Nullable *entries;
    size_t count;
    size_t capacity;
};

CosDictObj * COS_Nullable
cos_dict_obj_create(void)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

bool
cos_dict_obj_set(CosDictObj *dict_obj,
                 CosBaseObj *key,
                 CosBaseObj *value,
                 CosError * COS_Nullable error);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_DICT_OBJ_H */
