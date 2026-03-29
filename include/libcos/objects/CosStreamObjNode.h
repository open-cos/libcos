/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_OBJECTS_COS_STREAM_OBJ_NODE_H
#define LIBCOS_OBJECTS_COS_STREAM_OBJ_NODE_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

void
cos_stream_obj_node_destroy(CosStreamObjNode *stream_obj)
    COS_DEALLOCATOR_FUNC;

CosStreamObjNode * COS_Nullable
cos_stream_obj_node_create(CosDictObjNode *dict,
                      CosData * COS_Nullable data)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_stream_obj_node_destroy)
    COS_OWNERSHIP_TAKES(1, 2);

CosDictObjNode * COS_Nullable
cos_stream_obj_node_get_dict(const CosStreamObjNode *stream_obj);

CosData * COS_Nullable
cos_stream_obj_node_get_data(const CosStreamObjNode *stream_obj);

/**
 * @brief Get the length in bytes of the encoded stream data.
 *
 * @param stream_obj The stream object.
 *
 * @return The length in bytes of the encoded stream data.
 */
size_t
cos_stream_obj_node_get_length(const CosStreamObjNode *stream_obj);

/**
 * @brief Get the filter names for the stream.
 *
 * @param stream_obj The stream object.
 * @param out_error The error information.
 *
 * @return The filter names for the stream, or @c NULL if an error occurred.
 */
CosArrayObjNode * COS_Nullable
cos_stream_obj_node_get_filter_names(const CosStreamObjNode *stream_obj,
                                CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(2);

/**
 * @brief Get the length in bytes of the decoded stream data.
 *
 * @param stream_obj The stream object.
 * @param out_length_hint The output parameter for the length in bytes of the decoded stream data.
 * @param out_error The error information.
 *
 * @return @c true if the length hint was successfully retrieved, @c false otherwise.
 */
bool
cos_stream_obj_node_get_decoded_length_hint(const CosStreamObjNode *stream_obj,
                                       size_t *out_length_hint,
                                       CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(2)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_OBJECTS_COS_STREAM_OBJ_NODE_H */
