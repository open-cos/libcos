/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_OBJECTS_COS_STREAM_OBJ_H
#define LIBCOS_OBJECTS_COS_STREAM_OBJ_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

CosStreamObj * COS_Nullable
cos_stream_obj_alloc(CosDictObj *dict,
                     CosData * COS_Nullable data)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

CosDictObj * COS_Nullable
cos_stream_obj_get_dict(const CosStreamObj *stream_obj);

CosData * COS_Nullable
cos_stream_obj_get_data(const CosStreamObj *stream_obj);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_OBJECTS_COS_STREAM_OBJ_H */
