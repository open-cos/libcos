/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_COS_OBJ_H
#define LIBCOS_COS_OBJ_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>
#include <libcos/objects/CosDictObjNode.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * The type of a PDF object.
 *
 * Unlike @c CosObjNodeType, this enum does not include the indirect or
 * reference types -- those are resolved transparently by @c CosObj.
 */
typedef enum CosObjType {
    CosObjType_Unknown = 0,

    CosObjType_Boolean,
    CosObjType_Integer,
    CosObjType_Real,
    CosObjType_String,
    CosObjType_Name,
    CosObjType_Array,
    CosObjType_Dict,
    CosObjType_Stream,
    CosObjType_Null,
} CosObjType;

// MARK: - Lifecycle

/**
 * Destroys an object, releasing the retained object node.
 *
 * @param obj The object to destroy.
 */
void
cos_obj_destroy(CosObj *obj)
    COS_DEALLOCATOR_FUNC;

/**
 * Creates a new object wrapping the given object node.
 *
 * The node is retained as-is, preserving whether it is direct or indirect.
 * Type-specific accessors resolve through indirect and reference wrappers
 * transparently.
 *
 * @param node The object node to wrap.
 *
 * @return The new object, or @c NULL on allocation failure.
 */
CosObj * COS_Nullable
cos_obj_create(CosObjNode *node)
    COS_ALLOCATOR_FUNC
    COS_ALLOCATOR_FUNC_MATCHED_DEALLOC(cos_obj_destroy);

// MARK: - Type query

/**
 * Gets the resolved type of the object.
 *
 * For indirect and reference objects, this returns the type of the
 * underlying direct object (e.g. @c CosObjType_Integer for an indirect
 * integer).
 *
 * @param obj The object.
 *
 * @return The resolved object type.
 */
CosObjType
cos_obj_get_type(const CosObj *obj);

/**
 * Determines whether the object is direct.
 *
 * @param obj The object.
 *
 * @return @c true if the underlying node is a direct object.
 */
bool
cos_obj_is_direct(const CosObj *obj);

/**
 * Determines whether the object is indirect.
 *
 * Returns @c true if the underlying node is an indirect object definition
 * or an object reference.
 *
 * @param obj The object.
 *
 * @return @c true if the underlying node is indirect.
 */
bool
cos_obj_is_indirect(const CosObj *obj);

/**
 * Determines whether the object is null.
 *
 * @param obj The object.
 *
 * @return @c true if the resolved object is null.
 */
bool
cos_obj_is_null(const CosObj *obj);

// MARK: - Scalar accessors

/**
 * Gets the boolean value.
 *
 * @pre @c cos_obj_get_type returns @c CosObjType_Boolean.
 *
 * @param obj The object.
 *
 * @return The boolean value, or @c false on type mismatch.
 */
bool
cos_obj_get_bool_value(const CosObj *obj);

/**
 * Gets the integer value.
 *
 * @pre @c cos_obj_get_type returns @c CosObjType_Integer.
 *
 * @param obj The object.
 *
 * @return The integer value, or @c 0 on type mismatch.
 */
int
cos_obj_get_int_value(const CosObj *obj);

/**
 * Gets the real number value.
 *
 * @pre @c cos_obj_get_type returns @c CosObjType_Real.
 *
 * @param obj The object.
 *
 * @return The real value, or @c 0.0 on type mismatch.
 */
double
cos_obj_get_real_value(const CosObj *obj);

// MARK: - String / Name accessors

/**
 * Gets the string data.
 *
 * @pre @c cos_obj_get_type returns @c CosObjType_String.
 *
 * @param obj The object.
 *
 * @return The string data (borrowed), or @c NULL on type mismatch.
 */
const CosData * COS_Nullable
cos_obj_get_string_value(const CosObj *obj);

/**
 * Gets the name value.
 *
 * @pre @c cos_obj_get_type returns @c CosObjType_Name.
 *
 * @param obj The object.
 *
 * @return The name string (borrowed), or @c NULL on type mismatch.
 */
const CosString * COS_Nullable
cos_obj_get_name_value(const CosObj *obj);

// MARK: - Array accessors

/**
 * Gets the number of elements in the array.
 *
 * @pre @c cos_obj_get_type returns @c CosObjType_Array.
 *
 * @param obj The object.
 *
 * @return The element count, or @c 0 on type mismatch.
 */
size_t
cos_obj_get_array_count(const CosObj *obj);

/**
 * Gets the element at the given index.
 *
 * @pre @c cos_obj_get_type returns @c CosObjType_Array.
 *
 * The caller owns the returned object and must destroy it with
 * @c cos_obj_destroy.
 *
 * @param obj The object.
 * @param index The zero-based element index.
 * @param out_error On failure, receives error information.
 *
 * @return The element (owned), or @c NULL on failure.
 */
CosObj * COS_Nullable
cos_obj_get_object_at_index(const CosObj *obj,
                            size_t index,
                            CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

// MARK: - Dict accessors

/**
 * Gets the number of entries in the dictionary.
 *
 * @pre @c cos_obj_get_type returns @c CosObjType_Dict.
 *
 * @param obj The object.
 *
 * @return The entry count, or @c 0 on type mismatch.
 */
size_t
cos_obj_get_dict_count(const CosObj *obj);

/**
 * Gets the value for a key in the dictionary.
 *
 * @pre @c cos_obj_get_type returns @c CosObjType_Dict.
 *
 * The caller owns the returned object and must destroy it with
 * @c cos_obj_destroy.
 *
 * @param obj The object.
 * @param key The key, as a nul-terminated C string.
 * @param out_error On failure, receives error information.
 *
 * @return The value (owned), or @c NULL if the key was not found.
 */
CosObj * COS_Nullable
cos_obj_get_value_for_key(const CosObj *obj,
                          const char *key,
                          CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

// MARK: - Dict iterator

/**
 * An iterator over the key-value pairs of a dictionary object.
 *
 * Initialize with @c cos_obj_dict_iterator_init and advance with
 * @c cos_obj_dict_iterator_next. The iterator does not own any resources
 * and becomes invalid if the dictionary is mutated.
 */
typedef struct CosObjDictIterator {
    /** @private */
    CosDictObjNodeIterator base;
} CosObjDictIterator;

/**
 * Initializes a dictionary iterator.
 *
 * @pre @c cos_obj_get_type returns @c CosObjType_Dict.
 *
 * @param obj The dictionary object.
 *
 * @return An iterator positioned before the first entry.
 */
CosObjDictIterator
cos_obj_dict_iterator_init(const CosObj *obj);

/**
 * Advances the iterator to the next key-value pair.
 *
 * @param iterator The iterator.
 * @param[out] out_key On success, receives the key as a borrowed
 *             nul-terminated C string. Valid as long as the dictionary
 *             is alive.
 * @param[out] out_value On success, receives the value as an owned
 *             @c CosObj. The caller must destroy it with @c cos_obj_destroy.
 *
 * @return @c true if another entry was found, @c false when done.
 */
bool
cos_obj_dict_iterator_next(CosObjDictIterator *iterator,
                           const char * COS_Nullable * COS_Nonnull out_key,
                           CosObj * COS_Nullable * COS_Nonnull out_value)
    COS_ATTR_ACCESS_WRITE_ONLY(2)
    COS_ATTR_ACCESS_WRITE_ONLY(3);

// MARK: - Stream accessors

/**
 * Gets the stream dictionary.
 *
 * @pre @c cos_obj_get_type returns @c CosObjType_Stream.
 *
 * The caller owns the returned object and must destroy it with
 * @c cos_obj_destroy.
 *
 * @param obj The object.
 *
 * @return The stream dictionary (owned), or @c NULL on type mismatch.
 */
CosObj * COS_Nullable
cos_obj_get_stream_dict(const CosObj *obj);

/**
 * Gets the encoded stream data.
 *
 * @pre @c cos_obj_get_type returns @c CosObjType_Stream.
 *
 * @param obj The object.
 *
 * @return The stream data (borrowed), or @c NULL on type mismatch.
 */
const CosData * COS_Nullable
cos_obj_get_stream_data(const CosObj *obj);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_OBJ_H */
