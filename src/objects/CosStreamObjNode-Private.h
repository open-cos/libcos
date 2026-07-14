/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_OBJECTS_COS_STREAM_OBJ_NODE_PRIVATE_H
#define LIBCOS_OBJECTS_COS_STREAM_OBJ_NODE_PRIVATE_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * Detaches and returns the stream's dictionary, transferring ownership to the caller.
 *
 * After this call the stream node no longer owns the dictionary and will not destroy it. Used
 * to reuse an xref stream's dictionary as a trailer once the stream node is released.
 *
 * @param stream_obj The stream object.
 *
 * @return The dictionary (now owned by the caller), or @c NULL if it was already taken.
 */
CosDictObjNode * COS_Nullable
cos_stream_obj_node_take_dict_(CosStreamObjNode *stream_obj);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_OBJECTS_COS_STREAM_OBJ_NODE_PRIVATE_H */
