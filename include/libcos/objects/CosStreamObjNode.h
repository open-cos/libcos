/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_OBJECTS_COS_STREAM_OBJ_NODE_H
#define LIBCOS_OBJECTS_COS_STREAM_OBJ_NODE_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

COS_API void
cos_stream_obj_node_destroy(CosStreamObjNode *stream_obj)
    COS_DEALLOCATOR_FUNC;

/**
 * @brief Creates a stream object backed by its raw encoded bytes.
 *
 * @param dict The stream dictionary (ownership transferred).
 * @param encoded The raw (still-filtered) encoded bytes as a seekable stream (ownership
 *                transferred; closed with the node). In-memory callers can wrap a buffer with
 *                @c cos_memory_stream_create_readonly ; a stream parsed from a file is typically
 *                backed by a @c CosSubStream window over the file.
 *
 * @return A new stream object, or @c NULL on failure (in which case @p encoded is closed).
 */
COS_API CosStreamObjNode * COS_Nullable
cos_stream_obj_node_create(CosDictObjNode *dict,
                      CosStream *encoded)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_stream_obj_node_destroy)
    COS_OWNERSHIP_TAKES(1, 2);

COS_API CosDictObjNode * COS_Nullable
cos_stream_obj_node_get_dict(const CosStreamObjNode *stream_obj);

COS_API CosData * COS_Nullable
cos_stream_obj_node_get_data(const CosStreamObjNode *stream_obj);

/**
 * @brief Get the length in bytes of the encoded stream data.
 *
 * @param stream_obj The stream object.
 *
 * @return The length in bytes of the encoded stream data.
 */
COS_API size_t
cos_stream_obj_node_get_length(const CosStreamObjNode *stream_obj);

/**
 * @brief Get the filter names for the stream.
 *
 * @param stream_obj The stream object.
 * @param out_error The error information.
 *
 * @return The filter names for the stream, or @c NULL if an error occurred.
 */
COS_API CosArrayObjNode * COS_Nullable
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
COS_API bool
cos_stream_obj_node_get_decoded_length_hint(const CosStreamObjNode *stream_obj,
                                       size_t *out_length_hint,
                                       CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(2)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

/**
 * @brief Creates a stream over the decoded contents of the stream object.
 *
 * Builds a chain of decode filters from the stream's @c /Filter entry (a single
 * name or an array of names, in application order) over the encoded data. Reading
 * from the returned stream yields the decoded bytes; the caller owns the stream
 * and must close it with @c cos_stream_close.
 *
 * A stream with no @c /Filter yields its raw bytes. Unsupported filters, or a
 * @c /DecodeParms predictor that cannot yet be applied, cause a @c NULL return
 * with @p out_error set.
 *
 * @param stream_obj The stream object.
 * @param out_error On failure, set to describe the error.
 *
 * @return A new decode stream (owned), or @c NULL on error.
 */
COS_API CosStream * COS_Nullable
cos_stream_obj_node_create_decode_stream(const CosStreamObjNode *stream_obj,
                                    CosError * COS_Nullable out_error)
    COS_OWNERSHIP_RETURNS
    COS_ATTR_ACCESS_WRITE_ONLY(2);

/**
 * @brief Gets the fully decoded contents of the stream object.
 *
 * Convenience wrapper over @c cos_stream_obj_node_create_decode_stream that drains
 * the decode chain into a single buffer.
 *
 * @param stream_obj The stream object.
 * @param out_error On failure, set to describe the error.
 *
 * @return The decoded data (owned; free with @c cos_data_free), or @c NULL on error.
 */
COS_API CosData * COS_Nullable
cos_stream_obj_node_get_decoded_data(const CosStreamObjNode *stream_obj,
                                CosError * COS_Nullable out_error)
    COS_OWNERSHIP_RETURNS
    COS_ATTR_ACCESS_WRITE_ONLY(2);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_OBJECTS_COS_STREAM_OBJ_NODE_H */
