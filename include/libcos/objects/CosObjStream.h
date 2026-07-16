/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_OBJECTS_COS_OBJ_STREAM_H
#define LIBCOS_OBJECTS_COS_OBJ_STREAM_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * An object stream (a stream object whose dictionary declares @c /Type @c /ObjStm ) holds a
 * collection of other, "compressed", indirect objects. Its decoded contents are a header of
 * @c /N pairs of integers (object number and byte offset relative to @c /First ) followed at
 * byte @c /First by the @c /N contained objects, stored as bare direct objects (no
 * @c obj / @c endobj wrapper; their generation number is always zero).
 *
 * A @c CosObjStream decodes such a stream once and provides random access to the objects it
 * contains by index.
 *
 * An object stream may carry an @c /Extends entry referring to another object stream that it
 * extends. This is treated as metadata only: it is exposed via @c cos_obj_stream_get_extends ,
 * but the chain is not followed. Following it is unnecessary for resolving objects, because a
 * cross-reference entry for a compressed object names the object stream containing it directly.
 */

COS_API void
cos_obj_stream_destroy(CosObjStream *obj_stream)
    COS_DEALLOCATOR_FUNC;

/**
 * Creates an object stream from a stream object.
 *
 * The stream's @c /Type is verified to be @c /ObjStm , its @c /N and @c /First entries are
 * read, and its contents are decoded into a private buffer, so the returned object stream does
 * not depend on @p stream_obj afterwards.
 *
 * @param stream_obj The stream object (an @c /ObjStm ). Borrowed.
 * @param doc The owning document. Borrowed; used to parse the contained objects.
 * @param out_error On failure, set to describe the error.
 *
 * @return A new object stream (owned), or @c NULL on error.
 */
COS_API CosObjStream * COS_Nullable
cos_obj_stream_create(const CosStreamObjNode *stream_obj,
                      CosDoc *doc,
                      CosError * COS_Nullable out_error)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_obj_stream_destroy)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

/**
 * Gets the number of objects (@c /N ) in the object stream.
 *
 * @param obj_stream The object stream.
 *
 * @return The number of contained objects.
 */
COS_API size_t
cos_obj_stream_get_count(const CosObjStream *obj_stream);

/**
 * Gets the reference to the object stream that @p obj_stream extends (its @c /Extends entry).
 *
 * The @c /Extends chain is not followed; the reference is reported as metadata for callers
 * that want it.
 *
 * @param obj_stream The object stream.
 *
 * @return The referenced object stream's identifier, or @c CosObjID_Invalid if @p obj_stream
 *         has no (valid) @c /Extends entry. Test with @c cos_obj_id_is_valid .
 */
COS_API CosObjID
cos_obj_stream_get_extends(const CosObjStream *obj_stream)
    COS_ATTR_PURE
    COS_WARN_UNUSED_RESULT;

/**
 * Gets the object number of the contained object at @p index.
 *
 * @param obj_stream The object stream.
 * @param index The zero-based index into the object stream, in @c [0, count) .
 * @param out_obj_number The output parameter for the object number.
 * @param out_error On failure, set to describe the error.
 *
 * @return @c true on success, @c false if @p index is out of range.
 */
COS_API bool
cos_obj_stream_get_obj_number_at(const CosObjStream *obj_stream,
                                 size_t index,
                                 unsigned int *out_obj_number,
                                 CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(3)
    COS_ATTR_ACCESS_WRITE_ONLY(4);

/**
 * Parses and returns the contained object at @p index.
 *
 * @param obj_stream The object stream.
 * @param index The zero-based index into the object stream, in @c [0, count) .
 * @param out_error On failure, set to describe the error.
 *
 * @return The parsed object (owned by the caller; release with @c cos_obj_node_release ), or
 *         @c NULL on error.
 */
COS_API CosObjNode * COS_Nullable
cos_obj_stream_get_object_at(CosObjStream *obj_stream,
                             size_t index,
                             CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_OBJECTS_COS_OBJ_STREAM_H */
